#pragma once

#include <memory>

#include <QMainWindow>

#include "config/Settings.h"

class QComboBox;
class QLabel;
class QPushButton;
class QTimer;
class QCheckBox;
class QSlider;

namespace kameraa {

class Pipeline;
class OpenCvCaptureSource;
class IVirtualCamera;
class DenoiseFilter;
class RelightFilter;
class SharpenFilter;
class SuperResFilter;
class BackgroundFilter;
class FaceEnhanceFilter;
class FrameInterpolator;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onStartStop();
    void onRefreshCameras();
    void onPresetChanged(int index);
    void onEffectControlsChanged();
    void onAbout();
    void updatePreview();

private:
    void buildUi();
    QWidget* buildEffectsPanel();
    void refreshVirtualCameraStatus();
    void loadAiModels();
    void applySettingsToUi(const FilterSettings& f);
    void applyUiToFilters();
    void persistSettings();

    QComboBox* cameraCombo_ = nullptr;
    QComboBox* presetCombo_ = nullptr;
    QPushButton* startStopBtn_ = nullptr;
    QPushButton* refreshBtn_ = nullptr;
    QCheckBox* vcamCheck_ = nullptr;
    QLabel* previewLabel_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QLabel* vcamStatusLabel_ = nullptr;
    QTimer* previewTimer_ = nullptr;

    // Effect controls.
    QCheckBox* denoiseCheck_ = nullptr;
    QSlider* denoiseSlider_ = nullptr;
    QCheckBox* relightCheck_ = nullptr;
    QCheckBox* autoLightCheck_ = nullptr;
    QSlider* brightnessSlider_ = nullptr;
    QSlider* contrastSlider_ = nullptr;
    QSlider* saturationSlider_ = nullptr;
    QCheckBox* sharpenCheck_ = nullptr;
    QSlider* sharpenSlider_ = nullptr;
    QCheckBox* superresCheck_ = nullptr;
    QComboBox* superresScaleCombo_ = nullptr;
    QCheckBox* backgroundCheck_ = nullptr;
    QComboBox* backgroundModeCombo_ = nullptr;
    QSlider* backgroundSlider_ = nullptr;
    QCheckBox* faceCheck_ = nullptr;
    QSlider* faceSlider_ = nullptr;
    QCheckBox* interpCheck_ = nullptr;
    QComboBox* accelCombo_ = nullptr;
    QLabel* aiStatusLabel_ = nullptr;

    std::shared_ptr<OpenCvCaptureSource> capture_;
    std::shared_ptr<IVirtualCamera> vcam_;
    std::shared_ptr<DenoiseFilter> denoise_;
    std::shared_ptr<RelightFilter> relight_;
    std::shared_ptr<SharpenFilter> sharpen_;
    std::shared_ptr<SuperResFilter> superres_;
    std::shared_ptr<BackgroundFilter> background_;
    std::shared_ptr<FaceEnhanceFilter> face_;
    std::shared_ptr<FrameInterpolator> interp_;
    std::unique_ptr<Pipeline> pipeline_;

    Settings settings_;
    bool loadingUi_ = false;  // guards slots during programmatic UI updates
    bool running_ = false;
};

}  // namespace kameraa
