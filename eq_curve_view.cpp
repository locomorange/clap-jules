#include "eq_curve_view.h"
#include <vstgui/lib/cdrawcontext.h>
#include <vstgui/lib/cframe.h>
#include <cmath>
#include <algorithm>

EQCurveView::EQCurveView(const CRect& size)
    : CView(size)
    , dragging_point_index(-1)
    , is_dragging(false)
    , min_frequency(20.0)
    , max_frequency(20000.0)
    , min_gain(-12.0)
    , max_gain(12.0)
{
    // デフォルトのEQポイントを設定 (Set default EQ points)
    eq_points.push_back(EQPoint(200.0, 0.0));
    eq_points.push_back(EQPoint(1000.0, 0.0));
    eq_points.push_back(EQPoint(5000.0, 0.0));
    
    // 初期状態をUndoスタックに保存 (Save initial state to undo stack)
    saveStateForUndo();
}

EQCurveView::~EQCurveView() = default;

void EQCurveView::draw(CDrawContext* context) {
    // 背景を描画 (Draw background)
    context->setFillColor(CColor(25, 28, 32, 255));  // ダークグレー背景 (Dark gray background)
    context->drawRect(getViewSize(), kDrawFilled);
    
    // グリッドを描画 (Draw grid)
    drawGrid(context);
    
    // EQカーブを描画 (Draw EQ curve)
    drawCurve(context);
    
    // EQポイントを描画 (Draw EQ points)
    drawPoints(context);
    
    // ラベルを描画 (Draw labels)
    drawFrequencyLabels(context);
    drawGainLabels(context);
    
    // 境界線を描画 (Draw border)
    context->setLineWidth(1.0);
    context->setFrameColor(CColor(60, 65, 75, 255));
    context->drawRect(getViewSize(), kDrawStroked);
}

CMouseEventResult EQCurveView::onMouseDown(CPoint& where, const CButtonState& buttons) {
    if (buttons.isLeftButton()) {
        // クリック位置のポイントを探す (Find point at click position)
        int point_index = findPointAtLocation(where);
        
        if (point_index >= 0) {
            // 既存ポイントをドラッグ開始 (Start dragging existing point)
            dragging_point_index = point_index;
            is_dragging = true;
            
            // 他のポイントの選択を解除し、このポイントを選択 (Deselect other points and select this one)
            for (auto& point : eq_points) {
                point.is_selected = false;
            }
            eq_points[point_index].is_selected = true;
            
            saveStateForUndo();
            invalid();
            return kMouseEventHandled;
        } else if (buttons.isDoubleClick()) {
            // ダブルクリックで新しいポイントを追加 (Add new point on double click)
            auto freq_gain = pointToFrequencyGain(where);
            addEQPoint(freq_gain.first, freq_gain.second);
            invalid();
            return kMouseEventHandled;
        }
    } else if (buttons.isRightButton()) {
        // 右クリックでポイントを削除 (Remove point on right click)
        int point_index = findPointAtLocation(where);
        if (point_index >= 0 && eq_points.size() > 1) {  // 最低1つのポイントは残す (Keep at least one point)
            removeEQPoint(point_index);
            invalid();
            return kMouseEventHandled;
        }
    }
    
    return kMouseEventNotHandled;
}

CMouseEventResult EQCurveView::onMouseMoved(CPoint& where, const CButtonState& buttons) {
    if (is_dragging && dragging_point_index >= 0) {
        // ポイントをドラッグ (Drag point)
        auto freq_gain = pointToFrequencyGain(where);
        
        // 範囲内にクランプ (Clamp to range)
        freq_gain.first = std::max(min_frequency, std::min(max_frequency, freq_gain.first));
        freq_gain.second = std::max(min_gain, std::min(max_gain, freq_gain.second));
        
        eq_points[dragging_point_index].frequency = freq_gain.first;
        eq_points[dragging_point_index].gain = freq_gain.second;
        
        // パラメータ変更コールバックを呼び出し (Call parameter change callback)
        if (parameter_change_callback) {
            parameter_change_callback(dragging_point_index, freq_gain.first, freq_gain.second);
        }
        
        invalid();
        return kMouseEventHandled;
    }
    
    return kMouseEventNotHandled;
}

CMouseEventResult EQCurveView::onMouseUp(CPoint& where, const CButtonState& buttons) {
    if (is_dragging) {
        is_dragging = false;
        dragging_point_index = -1;
        return kMouseEventHandled;
    }
    
    return kMouseEventNotHandled;
}

void EQCurveView::addEQPoint(double frequency, double gain) {
    eq_points.emplace_back(frequency, gain);
    
    // 周波数順にソート (Sort by frequency)
    std::sort(eq_points.begin(), eq_points.end(), 
              [](const EQPoint& a, const EQPoint& b) { return a.frequency < b.frequency; });
    
    saveStateForUndo();
}

void EQCurveView::removeEQPoint(int index) {
    if (index >= 0 && index < static_cast<int>(eq_points.size()) && eq_points.size() > 1) {
        eq_points.erase(eq_points.begin() + index);
        saveStateForUndo();
    }
}

void EQCurveView::setEQPoint(int index, double frequency, double gain) {
    if (index >= 0 && index < static_cast<int>(eq_points.size())) {
        eq_points[index].frequency = frequency;
        eq_points[index].gain = gain;
    }
}

void EQCurveView::clearAllPoints() {
    eq_points.clear();
    eq_points.push_back(EQPoint(1000.0, 0.0));  // デフォルトポイントを1つ残す (Keep one default point)
    saveStateForUndo();
}

void EQCurveView::saveStateForUndo() {
    undo_stack.push_back(eq_points);
    redo_stack.clear();  // 新しい操作後はRedoスタックをクリア (Clear redo stack after new operation)
    
    // Undoスタックサイズを制限 (Limit undo stack size)
    if (undo_stack.size() > 50) {
        undo_stack.erase(undo_stack.begin());
    }
}

void EQCurveView::undo() {
    if (canUndo()) {
        redo_stack.push_back(eq_points);
        eq_points = undo_stack.back();
        undo_stack.pop_back();
        invalid();
    }
}

void EQCurveView::redo() {
    if (canRedo()) {
        undo_stack.push_back(eq_points);
        eq_points = redo_stack.back();
        redo_stack.pop_back();
        invalid();
    }
}

bool EQCurveView::canUndo() const {
    return !undo_stack.empty();
}

bool EQCurveView::canRedo() const {
    return !redo_stack.empty();
}

void EQCurveView::setParameterChangeCallback(std::function<void(int, double, double)> callback) {
    parameter_change_callback = callback;
}

void EQCurveView::updateFromParameters(const std::vector<std::pair<double, double>>& points) {
    eq_points.clear();
    for (const auto& point : points) {
        eq_points.emplace_back(point.first, point.second);
    }
    invalid();
}

void EQCurveView::setFrequencyRange(double min_freq, double max_freq) {
    min_frequency = min_freq;
    max_frequency = max_freq;
}

void EQCurveView::setGainRange(double min_g, double max_g) {
    min_gain = min_g;
    max_gain = max_g;
}

// プライベートメソッドの実装 (Private method implementations)

CPoint EQCurveView::frequencyGainToPoint(double frequency, double gain) const {
    CRect viewSize = getViewSize();
    
    // 対数スケールで周波数を変換 (Convert frequency with logarithmic scale)
    double log_freq = std::log(frequency);
    double log_min_freq = std::log(min_frequency);
    double log_max_freq = std::log(max_frequency);
    double x_norm = (log_freq - log_min_freq) / (log_max_freq - log_min_freq);
    
    // 線形スケールでゲインを変換 (Convert gain with linear scale)
    double y_norm = (gain - min_gain) / (max_gain - min_gain);
    
    return CPoint(
        viewSize.left + x_norm * viewSize.getWidth(),
        viewSize.top + (1.0 - y_norm) * viewSize.getHeight()  // Y軸は反転 (Y-axis is inverted)
    );
}

std::pair<double, double> EQCurveView::pointToFrequencyGain(const CPoint& point) const {
    CRect viewSize = getViewSize();
    
    double x_norm = (point.x - viewSize.left) / viewSize.getWidth();
    double y_norm = 1.0 - (point.y - viewSize.top) / viewSize.getHeight();  // Y軸は反転 (Y-axis is inverted)
    
    // 正規化値をクランプ (Clamp normalized values)
    x_norm = std::max(0.0, std::min(1.0, x_norm));
    y_norm = std::max(0.0, std::min(1.0, y_norm));
    
    // 対数スケールで周波数を計算 (Calculate frequency with logarithmic scale)
    double log_min_freq = std::log(min_frequency);
    double log_max_freq = std::log(max_frequency);
    double frequency = std::exp(log_min_freq + x_norm * (log_max_freq - log_min_freq));
    
    // 線形スケールでゲインを計算 (Calculate gain with linear scale)
    double gain = min_gain + y_norm * (max_gain - min_gain);
    
    return std::make_pair(frequency, gain);
}

int EQCurveView::findPointAtLocation(const CPoint& where, double tolerance) const {
    for (int i = 0; i < static_cast<int>(eq_points.size()); ++i) {
        CPoint point_pos = frequencyGainToPoint(eq_points[i].frequency, eq_points[i].gain);
        double distance = std::sqrt(std::pow(where.x - point_pos.x, 2) + std::pow(where.y - point_pos.y, 2));
        if (distance <= tolerance) {
            return i;
        }
    }
    return -1;
}

void EQCurveView::drawGrid(CDrawContext* context) const {
    context->setLineWidth(1.0);
    context->setFrameColor(CColor(45, 50, 58, 255));  // グリッドライン色 (Grid line color)
    
    CRect viewSize = getViewSize();
    
    // 垂直グリッドライン（周波数） (Vertical grid lines - frequency)
    std::vector<double> freq_lines = {50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
    for (double freq : freq_lines) {
        if (freq >= min_frequency && freq <= max_frequency) {
            CPoint point = frequencyGainToPoint(freq, 0.0);
            context->drawLine(CPoint(point.x, viewSize.top), CPoint(point.x, viewSize.bottom));
        }
    }
    
    // 水平グリッドライン（ゲイン） (Horizontal grid lines - gain)
    for (double gain = min_gain; gain <= max_gain; gain += 3.0) {
        CPoint point = frequencyGainToPoint(1000.0, gain);
        context->drawLine(CPoint(viewSize.left, point.y), CPoint(viewSize.right, point.y));
    }
}

void EQCurveView::drawCurve(CDrawContext* context) const {
    if (eq_points.empty()) return;
    
    std::vector<CPoint> curve_points;
    generateCurvePoints(curve_points);
    
    if (curve_points.size() < 2) return;
    
    // カーブの描画 (Draw curve)
    context->setLineWidth(2.0);
    context->setFrameColor(CColor(100, 150, 255, 255));  // 青色のカーブ (Blue curve)
    
    for (size_t i = 1; i < curve_points.size(); ++i) {
        context->drawLine(curve_points[i-1], curve_points[i]);
    }
}

void EQCurveView::drawPoints(CDrawContext* context) const {
    for (const auto& point : eq_points) {
        CPoint pos = frequencyGainToPoint(point.frequency, point.gain);
        
        // ポイントの背景円 (Point background circle)
        context->setFillColor(point.is_selected ? 
                             CColor(255, 120, 60, 255) :   // 選択時はオレンジ (Orange when selected)
                             CColor(255, 255, 255, 255));  // 通常は白 (White normally)
        
        CRect pointRect(pos.x - 4, pos.y - 4, pos.x + 4, pos.y + 4);
        context->drawEllipse(pointRect, kDrawFilled);
        
        // ポイントの境界線 (Point border)
        context->setLineWidth(1.5);
        context->setFrameColor(CColor(0, 0, 0, 255));
        context->drawEllipse(pointRect, kDrawStroked);
    }
}

void EQCurveView::drawFrequencyLabels(CDrawContext* context) const {
    // TODO: 周波数ラベルの描画実装 (TODO: Implement frequency label drawing)
    // 簡素化のため省略 (Omitted for simplicity)
}

void EQCurveView::drawGainLabels(CDrawContext* context) const {
    // TODO: ゲインラベルの描画実装 (TODO: Implement gain label drawing)
    // 簡素化のため省略 (Omitted for simplicity)
}

double EQCurveView::calculateResponseAtFrequency(double frequency) const {
    // 簡単な線形補間を使用してレスポンスを計算 (Calculate response using simple linear interpolation)
    if (eq_points.empty()) return 0.0;
    if (eq_points.size() == 1) return eq_points[0].gain;
    
    // 周波数順にソートされていることを前提 (Assume sorted by frequency)
    if (frequency <= eq_points.front().frequency) return eq_points.front().gain;
    if (frequency >= eq_points.back().frequency) return eq_points.back().gain;
    
    // 補間ポイントを見つける (Find interpolation points)
    for (size_t i = 1; i < eq_points.size(); ++i) {
        if (frequency <= eq_points[i].frequency) {
            double t = (frequency - eq_points[i-1].frequency) / 
                      (eq_points[i].frequency - eq_points[i-1].frequency);
            return eq_points[i-1].gain + t * (eq_points[i].gain - eq_points[i-1].gain);
        }
    }
    
    return 0.0;
}

void EQCurveView::generateCurvePoints(std::vector<CPoint>& curve_points) const {
    curve_points.clear();
    
    CRect viewSize = getViewSize();
    const int num_points = static_cast<int>(viewSize.getWidth());
    
    for (int x = 0; x < num_points; x += 2) {  // 2ピクセルごとにサンプリング (Sample every 2 pixels)
        CPoint screen_point(viewSize.left + x, 0);
        auto freq_gain = pointToFrequencyGain(screen_point);
        double response = calculateResponseAtFrequency(freq_gain.first);
        CPoint curve_point = frequencyGainToPoint(freq_gain.first, response);
        curve_points.push_back(curve_point);
    }
}