#pragma once

#include <vstgui/lib/cview.h>
#include <vstgui/lib/cdrawcontext.h>
#include <vstgui/lib/controls/ccontrol.h>
#include <vector>
#include <memory>

using namespace VSTGUI;

// EQポイント構造体 (EQ Point structure)
struct EQPoint {
    double frequency;    // 周波数 (Frequency)
    double gain;        // ゲイン (Gain)
    bool is_selected;   // 選択状態 (Selection state)
    
    EQPoint(double freq = 1000.0, double g = 0.0) 
        : frequency(freq), gain(g), is_selected(false) {}
};

// EQカーブビュークラス (EQ Curve View class)
class EQCurveView : public CView {
public:
    EQCurveView(const CRect& size);
    virtual ~EQCurveView();
    
    // VSTGUI描画関数 (VSTGUI drawing functions)
    void draw(CDrawContext* context) override;
    
    // マウスイベント処理 (Mouse event handling)
    CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
    CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;
    CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;
    
    // EQ操作メソッド (EQ manipulation methods)
    void addEQPoint(double frequency, double gain);
    void removeEQPoint(int index);
    void setEQPoint(int index, double frequency, double gain);
    void clearAllPoints();
    
    // Undo/Redo機能 (Undo/Redo functionality)
    void saveStateForUndo();
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;
    
    // パラメータ更新コールバック (Parameter update callback)
    void setParameterChangeCallback(std::function<void(int, double, double)> callback);
    
    // 外部からのパラメータ更新 (External parameter updates)
    void updateFromParameters(const std::vector<std::pair<double, double>>& points);
    
    // 表示設定 (Display settings)
    void setFrequencyRange(double min_freq, double max_freq);
    void setGainRange(double min_gain, double max_gain);
    
    // TODO: 改善ポイント (TODO: Improvement points)
    // - フィルターQ値の追加 (Add filter Q values)
    // - 複数フィルタータイプサポート (Support multiple filter types)
    // - スペクトラムアナライザー表示 (Spectrum analyzer display)
    // - より高度なカーブ補間 (More advanced curve interpolation)
    
private:
    std::vector<EQPoint> eq_points;          // EQポイントリスト (EQ points list)
    std::vector<std::vector<EQPoint>> undo_stack;  // Undoスタック (Undo stack)
    std::vector<std::vector<EQPoint>> redo_stack;  // Redoスタック (Redo stack)
    
    int dragging_point_index;                // ドラッグ中のポイントインデックス (Index of point being dragged)
    bool is_dragging;                       // ドラッグ状態 (Dragging state)
    
    double min_frequency, max_frequency;     // 周波数範囲 (Frequency range)
    double min_gain, max_gain;              // ゲイン範囲 (Gain range)
    
    std::function<void(int, double, double)> parameter_change_callback;
    
    // 内部ヘルパーメソッド (Internal helper methods)
    CPoint frequencyGainToPoint(double frequency, double gain) const;
    std::pair<double, double> pointToFrequencyGain(const CPoint& point) const;
    int findPointAtLocation(const CPoint& where, double tolerance = 10.0) const;
    void drawGrid(CDrawContext* context) const;
    void drawCurve(CDrawContext* context) const;
    void drawPoints(CDrawContext* context) const;
    void drawFrequencyLabels(CDrawContext* context) const;
    void drawGainLabels(CDrawContext* context) const;
    
    // カーブ計算 (Curve calculation)
    double calculateResponseAtFrequency(double frequency) const;
    void generateCurvePoints(std::vector<CPoint>& curve_points) const;
};