#include "gui/MainWindow.h"

#include <algorithm>
#include <filesystem>

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <opencv2/imgproc.hpp>

#include "capture/OpenCvCaptureSource.h"
#include "core/Pipeline.h"
#include "kameraa/Version.h"
#include "output/VirtualCameraFactory.h"
#include "processing/BackgroundFilter.h"
#include "processing/DenoiseFilter.h"
#include "processing/FaceEnhanceFilter.h"
#include "processing/FrameInterpolator.h"
#include "processing/ModelRegistry.h"
#include "processing/OnnxModel.h"
#include "processing/RelightFilter.h"
#include "processing/SharpenFilter.h"
#include "processing/SuperResFilter.h"

namespace kameraa {

namespace {
// Map a [-1,1] or [0,1] parameter to/from an integer slider [smin,smax].
int toSlider(double value, double lo, double hi, int smin, int smax) {
    const double t = (value - lo) / (hi - lo);
    return smin + static_cast<int>(t * (smax - smin) + 0.5);
}
double fromSlider(int pos, double lo, double hi, int smin, int smax) {
    const double t = static_cast<double>(pos - smin) / (smax - smin);
    return lo + t * (hi - lo);
}
OnnxModel::Accel accelFromName(const std::string& name) {
    if (name == "CPU") return OnnxModel::Accel::Cpu;
    if (name == "CUDA") return OnnxModel::Accel::Cuda;
    if (name == "DirectML") return OnnxModel::Accel::DirectML;
    if (name == "CoreML") return OnnxModel::Accel::CoreML;
    return OnnxModel::Accel::Auto;
}
}  // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    vcam_ = createVirtualCamera();
    denoise_ = std::make_shared<DenoiseFilter>();
    relight_ = std::make_shared<RelightFilter>();
    sharpen_ = std::make_shared<SharpenFilter>();
    superres_ = std::make_shared<SuperResFilter>();
    background_ = std::make_shared<BackgroundFilter>();
    face_ = std::make_shared<FaceEnhanceFilter>();
    interp_ = std::make_shared<FrameInterpolator>();

    loadSettings(defaultSettingsPath(), settings_);  // ignore failure -> defaults
    OnnxModel::setGlobalAccel(accelFromName(settings_.accel));

    buildUi();
    loadAiModels();
    onRefreshCameras();
    refreshVirtualCameraStatus();

    const int idx = presetCombo_->findText(QString::fromStdString(settings_.preset));
    if (idx >= 0) presetCombo_->setCurrentIndex(idx);
    applySettingsToUi(settings_.filters);
    applyUiToFilters();

    previewTimer_ = new QTimer(this);
    connect(previewTimer_, &QTimer::timeout, this, &MainWindow::updatePreview);
    previewTimer_->start(33);  // ~30 fps preview
}

MainWindow::~MainWindow() {
    if (pipeline_) pipeline_->stop();
    persistSettings();
}

void MainWindow::buildUi() {
    setWindowTitle("Kameraa");

    auto* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About Kameraa", this, &MainWindow::onAbout);

    auto* central = new QWidget(this);
    auto* root = new QHBoxLayout(central);

    auto* left = new QVBoxLayout();
    previewLabel_ = new QLabel("No preview", central);
    previewLabel_->setMinimumSize(640, 360);
    previewLabel_->setAlignment(Qt::AlignCenter);
    previewLabel_->setStyleSheet("background:#111; color:#888;");
    left->addWidget(previewLabel_, 1);

    auto* controls = new QHBoxLayout();
    cameraCombo_ = new QComboBox(central);
    refreshBtn_ = new QPushButton("Refresh", central);
    startStopBtn_ = new QPushButton("Start", central);
    controls->addWidget(new QLabel("Camera:", central));
    controls->addWidget(cameraCombo_, 1);
    controls->addWidget(refreshBtn_);
    controls->addWidget(startStopBtn_);
    left->addLayout(controls);

    vcamCheck_ = new QCheckBox("Output to virtual camera", central);
    left->addWidget(vcamCheck_);
    vcamStatusLabel_ = new QLabel(central);
    vcamStatusLabel_->setWordWrap(true);
    vcamStatusLabel_->setStyleSheet("color:#888;");
    left->addWidget(vcamStatusLabel_);
    statusLabel_ = new QLabel("Idle", central);
    left->addWidget(statusLabel_);

    root->addLayout(left, 1);
    root->addWidget(buildEffectsPanel());
    setCentralWidget(central);

    connect(refreshBtn_, &QPushButton::clicked, this, &MainWindow::onRefreshCameras);
    connect(startStopBtn_, &QPushButton::clicked, this, &MainWindow::onStartStop);
    connect(vcamCheck_, &QCheckBox::toggled, this, &MainWindow::onEffectControlsChanged);
}

QWidget* MainWindow::buildEffectsPanel() {
    auto* panel = new QWidget(this);
    panel->setMaximumWidth(280);
    auto* v = new QVBoxLayout(panel);

    auto* presetRow = new QHBoxLayout();
    presetRow->addWidget(new QLabel("Preset:", panel));
    presetCombo_ = new QComboBox(panel);
    presetCombo_->addItems({"Lite", "Balanced", "Studio"});
    presetCombo_->setCurrentText("Balanced");
    presetRow->addWidget(presetCombo_, 1);
    v->addLayout(presetRow);

    auto makeSlider = [panel](int smin, int smax) {
        auto* s = new QSlider(Qt::Horizontal, panel);
        s->setRange(smin, smax);
        return s;
    };

    auto* dBox = new QGroupBox("Denoise", panel);
    auto* dForm = new QFormLayout(dBox);
    denoiseCheck_ = new QCheckBox("Enabled", dBox);
    denoiseSlider_ = makeSlider(0, 100);
    dForm->addRow(denoiseCheck_);
    dForm->addRow("Strength", denoiseSlider_);
    v->addWidget(dBox);

    auto* rBox = new QGroupBox("Relight / Color", panel);
    auto* rForm = new QFormLayout(rBox);
    relightCheck_ = new QCheckBox("Enabled", rBox);
    autoLightCheck_ = new QCheckBox("Auto light", rBox);
    brightnessSlider_ = makeSlider(-100, 100);
    contrastSlider_ = makeSlider(-100, 100);
    saturationSlider_ = makeSlider(-100, 100);
    rForm->addRow(relightCheck_);
    rForm->addRow(autoLightCheck_);
    rForm->addRow("Brightness", brightnessSlider_);
    rForm->addRow("Contrast", contrastSlider_);
    rForm->addRow("Saturation", saturationSlider_);
    v->addWidget(rBox);

    auto* sBox = new QGroupBox("Sharpen", panel);
    auto* sForm = new QFormLayout(sBox);
    sharpenCheck_ = new QCheckBox("Enabled", sBox);
    sharpenSlider_ = makeSlider(0, 100);
    sForm->addRow(sharpenCheck_);
    sForm->addRow("Amount", sharpenSlider_);
    v->addWidget(sBox);

    auto* aiBox = new QGroupBox("AI (experimental)", panel);
    auto* aiForm = new QFormLayout(aiBox);
    superresCheck_ = new QCheckBox("Super-resolution", aiBox);
    superresScaleCombo_ = new QComboBox(aiBox);
    superresScaleCombo_->addItems({"x2", "x3", "x4"});
    backgroundCheck_ = new QCheckBox("Background", aiBox);
    backgroundModeCombo_ = new QComboBox(aiBox);
    backgroundModeCombo_->addItems({"Blur", "Replace"});
    backgroundSlider_ = makeSlider(0, 100);
    faceCheck_ = new QCheckBox("Face smoothing", aiBox);
    faceSlider_ = makeSlider(0, 100);
    interpCheck_ = new QCheckBox("Frame interpolation", aiBox);
    accelCombo_ = new QComboBox(aiBox);
    accelCombo_->addItems({"Auto", "CPU", "CUDA", "DirectML", "CoreML"});
    aiForm->addRow(superresCheck_);
    aiForm->addRow("Scale", superresScaleCombo_);
    aiForm->addRow(backgroundCheck_);
    aiForm->addRow("Mode", backgroundModeCombo_);
    aiForm->addRow("BG blur", backgroundSlider_);
    aiForm->addRow(faceCheck_);
    aiForm->addRow("Face", faceSlider_);
    aiForm->addRow(interpCheck_);
    aiForm->addRow("Accel", accelCombo_);
    aiStatusLabel_ = new QLabel(aiBox);
    aiStatusLabel_->setWordWrap(true);
    aiStatusLabel_->setStyleSheet("color:#888;");
    aiForm->addRow(aiStatusLabel_);
    v->addWidget(aiBox);
    v->addStretch(1);

    connect(presetCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &MainWindow::onPresetChanged);
    for (QCheckBox* c : {denoiseCheck_, relightCheck_, autoLightCheck_, sharpenCheck_,
                         superresCheck_, backgroundCheck_, faceCheck_, interpCheck_}) {
        connect(c, &QCheckBox::toggled, this, &MainWindow::onEffectControlsChanged);
    }
    for (QSlider* s : {denoiseSlider_, brightnessSlider_, contrastSlider_,
                       saturationSlider_, sharpenSlider_, backgroundSlider_,
                       faceSlider_}) {
        connect(s, &QSlider::valueChanged, this, &MainWindow::onEffectControlsChanged);
    }
    connect(accelCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &MainWindow::onEffectControlsChanged);
    for (QComboBox* c : {superresScaleCombo_, backgroundModeCombo_}) {
        connect(c, qOverload<int>(&QComboBox::currentIndexChanged), this,
                &MainWindow::onEffectControlsChanged);
    }
    return panel;
}

void MainWindow::loadAiModels() {
    ModelRegistry registry;
    const bool hasManifest = registry.loadManifest();

    const ModelInfo sr = registry.get("superres");
    const ModelInfo bg = registry.get("background");
    const ModelInfo fd = registry.get("face_detect");
    const ModelInfo ip = registry.get("interp");
    bool srOk = false, bgOk = false;
    if (sr.found) srOk = superres_->loadModel(sr.path, sr.scale > 1 ? sr.scale : 2);
    if (bg.found) bgOk = background_->loadModel(bg.path, bg.inputSize > 0 ? bg.inputSize : 512);
    if (fd.found) face_->loadDetector(fd.path);  // optional; skin fallback otherwise
    bool ipOk = false;
    if (ip.found) ipOk = interp_->loadModel(ip.path);

    superresCheck_->setEnabled(srOk);
    superresScaleCombo_->setEnabled(srOk);
    backgroundCheck_->setEnabled(bgOk);
    backgroundModeCombo_->setEnabled(bgOk);
    backgroundSlider_->setEnabled(bgOk);
    // Face smoothing always works (classical); detector just improves targeting.
    faceCheck_->setEnabled(true);
    faceSlider_->setEnabled(true);
    interpCheck_->setEnabled(ipOk);

    QString msg;
    if (!hasManifest) {
        msg = QString("No models/manifest.json found in '%1'. AI filters disabled; "
                      "classical enhancement still works.")
                  .arg(QString::fromStdString(registry.modelsDir()));
    } else {
        msg = QString("Super-res: %1   |   Background: %2")
                  .arg(srOk ? "ready" : "unavailable",
                       bgOk ? "ready" : "unavailable");
        msg += QString("\nFace: %1")
                   .arg(QString::fromStdString(face_->statusText()));
        msg += QString("   |   Interp: %1").arg(ipOk ? "ready" : "unavailable");
        if (!srOk || !bgOk) {
            msg += "\nMissing .onnx files stay disabled. See docs/06-AI-MODELS.md.";
        }
    }
    aiStatusLabel_->setText(msg);
}

void MainWindow::refreshVirtualCameraStatus() {
    const QString backend = QString::fromStdString(vcam_->backendName());
    if (vcam_->isAvailable()) {
        vcamCheck_->setEnabled(true);
        vcamCheck_->setChecked(settings_.virtualCameraOutput);
        vcamStatusLabel_->setText(
            QString("Virtual camera backend: %1 (ready)").arg(backend));
    } else {
        vcamCheck_->setEnabled(false);
        vcamCheck_->setChecked(false);
        vcamStatusLabel_->setText(
            QString("Virtual camera backend: %1 (unavailable)\n%2")
                .arg(backend, QString::fromStdString(vcam_->availabilityHint())));
    }
}

void MainWindow::applySettingsToUi(const FilterSettings& f) {
    loadingUi_ = true;
    denoiseCheck_->setChecked(f.denoiseEnabled);
    denoiseSlider_->setValue(toSlider(f.denoiseStrength, 0.0, 1.0, 0, 100));
    relightCheck_->setChecked(f.relightEnabled);
    autoLightCheck_->setChecked(f.autoLight);
    brightnessSlider_->setValue(toSlider(f.brightness, -1.0, 1.0, -100, 100));
    contrastSlider_->setValue(toSlider(f.contrast, -1.0, 1.0, -100, 100));
    saturationSlider_->setValue(toSlider(f.saturation, -1.0, 1.0, -100, 100));
    sharpenCheck_->setChecked(f.sharpenEnabled);
    sharpenSlider_->setValue(toSlider(f.sharpenAmount, 0.0, 1.0, 0, 100));
    superresCheck_->setChecked(f.superresEnabled);
    superresScaleCombo_->setCurrentIndex(std::clamp(f.superresScale - 2, 0, 2));
    backgroundCheck_->setChecked(f.backgroundEnabled);
    backgroundModeCombo_->setCurrentIndex(f.backgroundMode == 1 ? 1 : 0);
    backgroundSlider_->setValue(toSlider(f.backgroundStrength, 0.0, 1.0, 0, 100));
    faceCheck_->setChecked(f.faceEnabled);
    faceSlider_->setValue(toSlider(f.faceStrength, 0.0, 1.0, 0, 100));
    interpCheck_->setChecked(f.interpEnabled);
    accelCombo_->setCurrentText(QString::fromStdString(settings_.accel));
    loadingUi_ = false;
}

void MainWindow::applyUiToFilters() {
    FilterSettings& f = settings_.filters;
    f.denoiseEnabled = denoiseCheck_->isChecked();
    f.denoiseStrength = fromSlider(denoiseSlider_->value(), 0.0, 1.0, 0, 100);
    f.relightEnabled = relightCheck_->isChecked();
    f.autoLight = autoLightCheck_->isChecked();
    f.brightness = fromSlider(brightnessSlider_->value(), -1.0, 1.0, -100, 100);
    f.contrast = fromSlider(contrastSlider_->value(), -1.0, 1.0, -100, 100);
    f.saturation = fromSlider(saturationSlider_->value(), -1.0, 1.0, -100, 100);
    f.sharpenEnabled = sharpenCheck_->isChecked();
    f.sharpenAmount = fromSlider(sharpenSlider_->value(), 0.0, 1.0, 0, 100);
    f.superresEnabled = superresCheck_->isChecked();
    f.superresScale = superresScaleCombo_->currentIndex() + 2;
    f.backgroundEnabled = backgroundCheck_->isChecked();
    f.backgroundMode = backgroundModeCombo_->currentIndex();
    f.backgroundStrength = fromSlider(backgroundSlider_->value(), 0.0, 1.0, 0, 100);
    if (vcamCheck_->isEnabled()) settings_.virtualCameraOutput = vcamCheck_->isChecked();

    denoise_->setEnabled(f.denoiseEnabled);
    denoise_->setStrength(f.denoiseStrength);
    relight_->setEnabled(f.relightEnabled);
    relight_->setAutoLight(f.autoLight);
    relight_->setBrightness(f.brightness);
    relight_->setContrast(f.contrast);
    relight_->setSaturation(f.saturation);
    sharpen_->setEnabled(f.sharpenEnabled);
    sharpen_->setAmount(f.sharpenAmount);
    superres_->setEnabled(f.superresEnabled && superres_->available());
    background_->setEnabled(f.backgroundEnabled && background_->available());
    background_->setMode(f.backgroundMode == 1 ? BackgroundFilter::Mode::Replace
                                               : BackgroundFilter::Mode::Blur);
    background_->setStrength(f.backgroundStrength);
    face_->setEnabled(f.faceEnabled);
    face_->setStrength(f.faceStrength);
    interp_->setEnabled(f.interpEnabled && interp_->isAvailable());
    settings_.accel = accelCombo_->currentText().toStdString();
}

void MainWindow::onPresetChanged(int /*index*/) {
    if (loadingUi_) return;
    settings_.preset = presetCombo_->currentText().toStdString();
    applySettingsToUi(presetByName(settings_.preset));
    applyUiToFilters();
}

void MainWindow::onEffectControlsChanged() {
    if (loadingUi_) return;
    applyUiToFilters();
}

void MainWindow::onAbout() {
    QMessageBox::about(
        this, "About Kameraa",
        QString("<b>%1</b> %2<br><br>"
                "AI virtual camera enhancer. Runs locally and offline.<br><br>"
                "Enhances your webcam (denoise, relight, upscale, background, face) "
                "and publishes it as a virtual camera for Teams, Meet, Zoom, etc.<br><br>"
                "<i>Note:</i> enhancement cannot create true 4K detail a sensor never "
                "captured, and call apps cap what they transmit (usually 1080p30).")
            .arg(kAppName, kVersion));
}

void MainWindow::onRefreshCameras() {
    cameraCombo_->clear();
    OpenCvCaptureSource probe;
    for (const auto& cam : probe.enumerate()) {
        cameraCombo_->addItem(QString::fromStdString(cam.name), cam.index);
    }
    if (cameraCombo_->count() == 0) {
        cameraCombo_->addItem("No camera found", -1);
    }
    const int idx = cameraCombo_->findData(settings_.cameraIndex);
    if (idx >= 0) cameraCombo_->setCurrentIndex(idx);
}

void MainWindow::onStartStop() {
    if (running_) {
        if (pipeline_) pipeline_->stop();
        running_ = false;
        startStopBtn_->setText("Start");
        statusLabel_->setText("Stopped");
        return;
    }

    const int index = cameraCombo_->currentData().toInt();
    if (index < 0) {
        statusLabel_->setText("No camera selected");
        return;
    }
    settings_.cameraIndex = index;

    capture_ = std::make_shared<OpenCvCaptureSource>();
    pipeline_ = std::make_unique<Pipeline>();
    pipeline_->setCapture(capture_);
    pipeline_->addFilter(denoise_);
    pipeline_->addFilter(relight_);
    pipeline_->addFilter(sharpen_);
    pipeline_->addFilter(superres_);
    pipeline_->addFilter(face_);
    pipeline_->addFilter(background_);
    pipeline_->setInterpolator(interp_);

    const bool useVcam = vcamCheck_->isChecked() && vcam_->isAvailable();
    if (useVcam) pipeline_->setVirtualCamera(vcam_);

    CaptureConfig in;
    in.index = index;
    OutputConfig out;
    pipeline_->setTargetFps(in.fps);

    if (!pipeline_->start(in, out)) {
        statusLabel_->setText(useVcam ? "Failed to start camera or virtual camera"
                                      : "Failed to open camera");
        pipeline_.reset();
        return;
    }

    running_ = true;
    startStopBtn_->setText("Stop");
    statusLabel_->setText(useVcam ? "Running (streaming to virtual camera)"
                                  : "Running");
}

void MainWindow::updatePreview() {
    if (!running_ || !pipeline_) return;

    Frame frame;
    if (!pipeline_->latestFrame(frame) || frame.empty()) return;

    cv::Mat rgb;
    cv::cvtColor(frame.image, rgb, cv::COLOR_BGR2RGB);
    QImage img(rgb.data, rgb.cols, rgb.rows,
               static_cast<int>(rgb.step), QImage::Format_RGB888);
    previewLabel_->setPixmap(QPixmap::fromImage(img.copy())
                                 .scaled(previewLabel_->size(),
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation));

    statusLabel_->setText(QString("Running  |  %1 fps  |  work %2%")
                              .arg(pipeline_->currentFps(), 0, 'f', 1)
                              .arg(pipeline_->currentWorkScale() * 100.0, 0, 'f', 0));
}

void MainWindow::persistSettings() {
    applyUiToFilters();  // sync UI -> settings_
    try {
        const std::string path = defaultSettingsPath();
        std::filesystem::create_directories(
            std::filesystem::path(path).parent_path());
        saveSettings(path, settings_);
    } catch (const std::exception&) {
        // Non-fatal: settings just won't persist this run.
    }
}

}  // namespace kameraa
