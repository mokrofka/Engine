
// #define UI_Window(begin) DeferLoop(begin, ImGui::End())
// #define UI_Window1(...) DeferLoop(ImGui::Begin(__VA_ARGS__), ImGui::End())
#define UI_Window(...) DeferLoop(ImGui::Begin(__VA_ARGS__), ImGui::End())

#define UI_TabBar(...) IfDeferLoop(ImGui::BeginTabBar(__VA_ARGS__), ImGui::EndTabBar())
#define UI_TabItem(...) IfDeferLoop(ImGui::BeginTabItem(__VA_ARGS__), ImGui::EndTabItem())
