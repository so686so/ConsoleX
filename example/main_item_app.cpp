#include "ConsoleX.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <optional>
#include <sstream>
#include <functional>

using namespace std::chrono_literals;

// =============================================================================
// [Main App Logic]
// =============================================================================

struct Rect {
    int x, y, w, h;
    bool Contains(int px, int py) const {
        return (px >= x && px < x + w && py >= y && py < y + h);
    }
    bool Intersects(const Rect& other) const {
        return (x < other.x + other.w && x + w > other.x &&
                y < other.y + other.h && y + h > other.y);
    }
};

struct Item {
    std::string name;
    std::string desc;
};

class Inventory {
public:
    std::string title;
    std::vector<Item> items;
    Rect rect;
    bool is_red_border = false;
    bool is_green_border = false;
    Rect saved_rect;

    Inventory(std::string t, int x, int y, int w) : title(t), rect{x, y, w, 0} {}

    int GetCalculatedHeight() const { return 3 + (int)items.size() + 1; }

    void UpdateHeight() { rect.h = GetCalculatedHeight(); }

    void SortItems() {
        std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
            return a.name < b.name;
        });
    }

    std::string TruncateText(const std::string& text, int max_width) {
        int w = (int)cx::Util::GetStringWidth(text);
        if (w <= max_width) return text;

        std::string res = text;
        while (res.length() > 0 && (int)cx::Util::GetStringWidth(res + "..") > max_width) {
            while (!res.empty()) {
                char c = res.back();
                res.pop_back();
                if ((c & 0xC0) != 0x80) break;
            }
        }
        return res + "..";
    }

    void DrawToBuffer(cx::Buffer& buffer) {
        cx::Color fg_c = cx::Color::White;
        if (is_red_border) fg_c = cx::Color::Red;
        else if (is_green_border) fg_c = cx::Color::Green;

        cx::Color bg_c = cx::Color::Black;

        buffer.DrawBox(rect.x, rect.y, rect.w, rect.h, fg_c, bg_c, is_red_border);
        buffer.DrawString(rect.x + 1, rect.y, "[-] ", fg_c, bg_c);

        buffer.DrawString(rect.x, rect.y + 2, "┣", fg_c, bg_c);
        buffer.DrawString(rect.x + rect.w - 1, rect.y + 2, "┫", fg_c, bg_c);
        for(int i = rect.x + 1; i < rect.x + rect.w - 1; ++i)
            buffer.DrawString(i, rect.y + 2, "━", fg_c, bg_c);

        int content_w = rect.w - 2;
        std::string display_title = TruncateText(title, content_w);
        int title_w = (int)cx::Util::GetStringWidth(display_title);
        int center_x = rect.x + (rect.w - title_w) / 2;
        buffer.DrawString(center_x, rect.y + 1, display_title, cx::Color::Yellow, bg_c);

        for(size_t i=0; i<items.size(); ++i) {
            int row_y = rect.y + 3 + (int)i;
            if (row_y >= rect.y + rect.h - 1) break;

            std::string prefix = std::to_string(i+1) + ". ";
            int prefix_w = (int)cx::Util::GetStringWidth(prefix);
            int item_space = content_w - prefix_w - 1;

            std::string item_name = TruncateText(items[i].name, item_space);
            std::string line = prefix + item_name;

            buffer.DrawString(rect.x + 2, row_y, line, cx::Color::White, bg_c);
        }
    }

    bool HitHandle(int px, int py) { return (py == rect.y && px >= rect.x && px <= rect.x + 4); }

    bool HitResize(int px, int py) {
        bool right_edge = (px == rect.x + rect.w - 1 && py >= rect.y && py < rect.y + rect.h);
        bool bottom_edge = (py == rect.y + rect.h - 1 && px >= rect.x && px < rect.x + rect.w);
        return right_edge || bottom_edge;
    }

    int HitItemIndex(int px, int py) {
        if (px > rect.x && px < rect.x + rect.w - 1) {
            int row = py - (rect.y + 3);
            if (row >= 0 && row < (int)items.size()) return row;
        }
        return -1;
    }
};

class InventoryApp {
    enum class DragMode { NONE, WINDOW_MOVE, WINDOW_RESIZE, ITEM_MOVE };
    enum class ViewMode { NORMAL, MAXIMIZED };

    bool is_running = true;
    bool need_render = true;
    ViewMode view_mode = ViewMode::NORMAL;

    std::vector<Inventory> inventories;
    cx::Buffer screen_buffer;

    DragMode drag_mode = DragMode::NONE;
    int drag_target_idx = -1;
    int drag_item_idx = -1;
    cx::Coord drag_offset = {0, 0};
    Item dragging_item;

    cx::Coord drag_start_win_pos = {0, 0};

    cx::Coord mouse_cursor = {0, 0};
    std::string log_msg = "Ready";

    struct MenuBox { int x, w; std::function<void()> action; };
    std::vector<MenuBox> menus;

public:
    InventoryApp() {
        for(int i=0; i<5; ++i) {
            std::string title = "Inventory " + std::string(1, 'A' + i);
            int x = 2 + (i * 32);
            int y = 5;
            inventories.emplace_back(title, x, y, 30);
        }

        int items_per_inv[] = {4, 3, 2, 1, 0};
        int total_item_cnt = 0;
        for(int i=0; i<5; ++i) {
            for(int k=0; k<items_per_inv[i]; ++k) {
                total_item_cnt++;
                std::string i_name = "Equipment_No." + std::to_string(total_item_cnt);
                std::string i_desc = "Desc for " + i_name;
                inventories[i].items.push_back({i_name, i_desc});
            }
        }

        SaveLayout();
    }

    void Run() {
        cx::Device::EnableMouse(true);

        cx::Screen::SetBackColor(cx::Color::Black); // 배경색을 검정으로 설정
        cx::Screen::Clear();                        // 해당 배경색으로 화면 전체 지우기
        std::cout << std::flush;                    // 즉시 반영

        while(is_running) {
            if(auto input = cx::Device::GetInput(10ms); input) {
                ProcessInput(cx::Device::Inspect(input));
                need_render = true;
            }

            if (need_render) {
                Render();
                need_render = false;
            }
        }
        cx::Device::EnableMouse(false);
        cx::Screen::Clear();
    }

private:
    void SaveLayout() { for(auto& inv : inventories) inv.saved_rect = inv.rect; }
    void RestoreLayout() {
        for(auto& inv : inventories) {
            inv.rect.x = inv.saved_rect.x;
            inv.rect.y = inv.saved_rect.y;
            inv.rect.w = inv.saved_rect.w;
        }
    }

    void ApplyMaximizedLayout() {
        auto size = cx::Screen::GetSize();
        int part_w = size.cols / 5;

        std::vector<std::pair<int, int>> sorted_indices;
        for(int i=0; i<(int)inventories.size(); ++i) {
            int score = inventories[i].rect.x * 1000 + inventories[i].rect.y;
            sorted_indices.push_back({score, i});
        }
        std::sort(sorted_indices.begin(), sorted_indices.end());

        for(int k=0; k<5; ++k) {
            int idx = sorted_indices[k].second;
            int x_pos = 1 + (k * part_w);
            inventories[idx].rect.x = x_pos;
            inventories[idx].rect.y = 2;
            inventories[idx].rect.w = part_w - 1;
            inventories[idx].rect.h = size.rows - 3;
        }
    }

    cx::Coord FindValidPosition(int target_idx, int start_x, int start_y) {
        auto& target = inventories[target_idx];
        int w = target.rect.w;
        int h = target.GetCalculatedHeight();
        auto size = cx::Screen::GetSize();
        for(int y = 2; y < size.rows - h; y += 2) {
            for(int x = 1; x < size.cols - w; x += 2) {
                Rect candidate { x, y, w, h };
                bool collision = false;
                for(int i=0; i<(int)inventories.size(); ++i) {
                    if(i == target_idx) continue;
                    if(candidate.Intersects(inventories[i].rect)) { collision = true; break; }
                }
                if(!collision) return {x, y};
            }
        }
        return {1, 2};
    }

    bool CheckCollisionSimple(int target_idx, const Rect& test_rect) {
        for(int i=0; i<(int)inventories.size(); ++i) {
            if(i == target_idx) continue;
            if(test_rect.Intersects(inventories[i].rect)) return true;
        }
        return false;
    }

    void CheckWindowCollision(int current_idx) {
        inventories[current_idx].is_red_border = false;
        if(view_mode == ViewMode::NORMAL && !(drag_mode == DragMode::WINDOW_RESIZE && drag_target_idx == current_idx)) {
             inventories[current_idx].UpdateHeight();
        }

        for(int i=0; i<(int)inventories.size(); ++i) {
            if(i == current_idx) continue;
            if(inventories[current_idx].rect.Intersects(inventories[i].rect)) {
                inventories[current_idx].is_red_border = true; break;
            }
        }
    }

    void ProcessInput(const cx::Device::Event& ev) {
        if(ev.code == cx::DeviceInputCode::MOUSE_EVENT) {
            mouse_cursor = { ev.mouse.x, ev.mouse.y };
        }

        if(ev.code == cx::DeviceInputCode::q) { is_running = false; return; }
        if(ev.code == cx::DeviceInputCode::F1) {
            if(view_mode == ViewMode::NORMAL) {
                SaveLayout(); ApplyMaximizedLayout(); view_mode = ViewMode::MAXIMIZED; log_msg = "Mode: Maximized";
            }
        }
        if(ev.code == cx::DeviceInputCode::F2) {
            if(view_mode == ViewMode::MAXIMIZED) {
                RestoreLayout(); view_mode = ViewMode::NORMAL; log_msg = "Mode: Normal";
            }
        }

        if(ev.code == cx::DeviceInputCode::MOUSE_EVENT) {
            int mx = mouse_cursor.x;
            int my = mouse_cursor.y;

            if(ev.mouse.button == cx::MouseButton::LEFT) {
                if(ev.mouse.action == cx::MouseAction::PRESS) {
                    if(my == 0) {
                        for(const auto& menu : menus) {
                            if(mx >= menu.x && mx < menu.x + menu.w) { menu.action(); return; }
                        }
                    }
                    for(int i=(int)inventories.size()-1; i>=0; --i) {
                        auto& inv = inventories[i];
                        if (view_mode == ViewMode::NORMAL) inv.UpdateHeight();

                        bool allow_window_ops = (view_mode == ViewMode::NORMAL);

                        if(allow_window_ops && inv.HitHandle(mx, my)) {
                            drag_mode = DragMode::WINDOW_MOVE; drag_target_idx = i;
                            drag_offset = { mx - inv.rect.x, my - inv.rect.y };
                            drag_start_win_pos = { inv.rect.x, inv.rect.y };
                            return;
                        }
                        if(allow_window_ops && inv.HitResize(mx, my)) {
                            drag_mode = DragMode::WINDOW_RESIZE; drag_target_idx = i; return;
                        }
                        int item_idx = inv.HitItemIndex(mx, my);
                        if(item_idx != -1) {
                            log_msg = "Selected: " + inv.items[item_idx].name;
                            drag_mode = DragMode::ITEM_MOVE; drag_target_idx = i;
                            drag_item_idx = item_idx; dragging_item = inv.items[item_idx]; return;
                        }
                    }
                }
                else if(ev.mouse.action == cx::MouseAction::DRAG) {
                    if(drag_mode == DragMode::WINDOW_MOVE) {
                        auto& inv = inventories[drag_target_idx];
                        inv.rect.x = mx - drag_offset.x; inv.rect.y = my - drag_offset.y;
                        CheckWindowCollision(drag_target_idx);
                    }
                    else if(drag_mode == DragMode::WINDOW_RESIZE) {
                        auto& inv = inventories[drag_target_idx];
                        int new_w = mx - inv.rect.x + 1;
                        int new_h = my - inv.rect.y + 1;

                        if (new_w < 15) new_w = 15;

                        // [Req] 아이템 목록 크기 미만으로 줄이지 못하게 제한
                        int min_h = inv.GetCalculatedHeight();
                        if (new_h < min_h) new_h = min_h;

                        Rect test_rect = inv.rect;
                        test_rect.w = new_w;
                        test_rect.h = new_h;

                        if(!CheckCollisionSimple(drag_target_idx, test_rect)) {
                            inv.rect.w = new_w;
                            inv.rect.h = new_h;
                        }
                    }
                    else if(drag_mode == DragMode::ITEM_MOVE) {
                        for(int i=0; i<(int)inventories.size(); ++i) {
                            inventories[i].is_green_border = false;
                            if (i == drag_target_idx) continue;

                            int check_h = inventories[i].rect.h;
                            if (view_mode == ViewMode::NORMAL) check_h = inventories[i].GetCalculatedHeight();

                            Rect check_rect = inventories[i].rect;
                            check_rect.h = check_h;

                            if(check_rect.Contains(mx, my)) {
                                inventories[i].is_green_border = true;
                            }
                        }
                    }
                }
                else if(ev.mouse.action == cx::MouseAction::RELEASE) {
                    if(drag_mode == DragMode::WINDOW_MOVE) {
                        auto& inv = inventories[drag_target_idx];

                        if (inv.rect.x == drag_start_win_pos.x && inv.rect.y == drag_start_win_pos.y) {
                            inv.SortItems();
                            log_msg = "Items Sorted: " + inv.title;
                        }
                        else {
                            auto size = cx::Screen::GetSize();
                            int clamped_x = std::clamp(inv.rect.x, 1, size.cols - inv.rect.w);
                            int clamped_y = std::clamp(inv.rect.y, 2, size.rows - inv.rect.h - 1);

                            inv.rect.x = clamped_x;
                            inv.rect.y = clamped_y;

                            if (CheckCollisionSimple(drag_target_idx, inv.rect)) {
                                inv.is_red_border = true;
                            } else {
                                inv.is_red_border = false;
                            }

                            if(inv.is_red_border) {
                                auto valid_pos = FindValidPosition(drag_target_idx, 1, 2);
                                inv.rect.x = valid_pos.x;
                                inv.rect.y = valid_pos.y;
                                inv.is_red_border = false;
                            }
                        }
                    }
                    else if(drag_mode == DragMode::WINDOW_RESIZE) {
                         // Release 시 높이 자동 복구는 Render 루프에서 처리
                    }
                    else if(drag_mode == DragMode::ITEM_MOVE) {
                        for(auto& inv : inventories) inv.is_green_border = false;

                        for(int i=0; i<(int)inventories.size(); ++i) {
                            Rect target_rect = inventories[i].rect;
                            if (view_mode == ViewMode::NORMAL) target_rect.h = inventories[i].GetCalculatedHeight();

                            if(i != drag_target_idx && target_rect.Contains(mx, my)) {
                                inventories[i].items.push_back(dragging_item);
                                inventories[drag_target_idx].items.erase(
                                    inventories[drag_target_idx].items.begin() + drag_item_idx
                                );
                                log_msg = "Moved to " + inventories[i].title;
                                break;
                            }
                        }
                    }
                    drag_mode = DragMode::NONE; drag_target_idx = -1; drag_item_idx = -1;
                }
            }
        }
    }

    void DrawTopBar(cx::Buffer& buffer) {
        menus.clear();
        cx::Color bg = cx::Color::Blue; cx::Color fg = cx::Color::White;
        auto size = cx::Screen::GetSize();
        for(int x=0; x<size.cols; ++x) buffer.DrawString(x, 0, " ", fg, bg);

        int current_x = 1;
        auto AddMenu = [&](std::string label, std::function<void()> act) {
            std::string txt = " " + label + " ";
            buffer.DrawString(current_x, 0, txt, fg, bg);
            menus.push_back({current_x, (int)cx::Util::GetStringWidth(txt), act});
            current_x += (int)cx::Util::GetStringWidth(txt);
            buffer.DrawString(current_x++, 0, "|", fg, bg);
        };

        AddMenu("[Q]uit", [&](){ is_running = false; });
        AddMenu("[F1] Max", [&](){ if(view_mode==ViewMode::NORMAL) { SaveLayout(); ApplyMaximizedLayout(); view_mode=ViewMode::MAXIMIZED; }});
        AddMenu("[F2] Restore", [&](){ if(view_mode==ViewMode::MAXIMIZED) { RestoreLayout(); view_mode=ViewMode::NORMAL; }});
    }

    void DrawBottomLog(cx::Buffer& buffer) {
        auto size = cx::Screen::GetSize();
        int y = size.rows - 1;
        cx::Color bg = cx::Color(40,40,40); cx::Color fg = cx::Color::White;
        for(int x=0; x<size.cols; ++x) buffer.DrawString(x, y, " ", fg, bg);
        std::string line = " Log: " + log_msg;
        buffer.DrawString(1, y, line, fg, bg);
    }

    void Render() {
        for(int i=0; i<(int)inventories.size(); ++i) {
            bool is_resizing_this = (drag_mode == DragMode::WINDOW_RESIZE && drag_target_idx == i);
            if (view_mode == ViewMode::NORMAL && !is_resizing_this) {
                inventories[i].UpdateHeight();
            }
        }

        auto size = cx::Screen::GetSize();
        screen_buffer.Resize(size.cols, size.rows);
        screen_buffer.Clear(cx::Color::Black);

        for(auto& inv : inventories) inv.DrawToBuffer(screen_buffer);
        DrawTopBar(screen_buffer);
        DrawBottomLog(screen_buffer);

        if(drag_mode == DragMode::ITEM_MOVE) {
            int x = mouse_cursor.x + 2; int y = mouse_cursor.y + 1;
            std::string content = " " + dragging_item.name + " ";
            int w = (int)cx::Util::GetStringWidth(content);
            cx::Color box_bg = cx::Color::Black; cx::Color box_fg = cx::Color::Cyan;

            std::stringstream ss; ss << "┌"; for(int i=0; i<w; ++i) ss << "─"; ss << "┐";
            screen_buffer.DrawString(x, y, ss.str(), box_fg, box_bg);
            screen_buffer.DrawString(x, y+1, "│", box_fg, box_bg);
            screen_buffer.DrawString(x+1, y+1, content, box_fg, box_bg);
            screen_buffer.DrawString(x+1+w, y+1, "│", box_fg, box_bg);
            ss.str(""); ss.clear(); ss << "└"; for(int i=0; i<w; ++i) ss << "─"; ss << "┘";
            screen_buffer.DrawString(x, y+2, ss.str(), box_fg, box_bg);
        }
        screen_buffer.Flush();
    }
};

int main() {
    InventoryApp app;
    app.Run();
    return 0;
}