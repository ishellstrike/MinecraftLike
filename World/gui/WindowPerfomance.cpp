#include "WindowPerfomance.h"
#include <tools\order_casters.h>
#include <tools\wset.h>

void WindowPerfomance::Draw(glm::vec2 mainwin_size)
{
  ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiSetCond_FirstUseEver);
  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
  ImGui::SetNextWindowCollapsed(true, ImGuiSetCond_FirstUseEver);
  
  ImGui::Begin("Perfomance monitor", &mOpen, ImGuiWindowFlags_AlwaysAutoResize);
  {
    ImGui::SetNextTreeNodeOpened(true, ImGuiSetCond_FirstUseEver);
    if (ImGui::TreeNode("mem"))
    {
      ImGui::LabelText("total", "%s", mems.c_str());
      ImGui::TreePop();
    }

    ImGui::SetNextTreeNodeOpened(true, ImGuiSetCond_FirstUseEver);
    if (ImGui::TreeNode("deltas"))
    {
      ImGui::LabelText("render dt", "%g ms", dt*1000);
      tess_perf.Draw();
      gen_perf.Draw();
      ImGui::TreePop();
    }

    ImGui::SetNextTreeNodeOpened(true, ImGuiSetCond_FirstUseEver);
    if (ImGui::TreeNode("render fps"))
    {
      ImGui::PlotLines(
        "",
        &fps_sec[0],
        100,
        fps_sec_cur, std::to_string(fps_sec[fps_sec_cur]).c_str(), 0, 1000, { 100,50 });

      ImGui::PlotLines(
        "",
        &fps_subsec[0],
        100,
        fps_subsec_cur, std::to_string(fps_subsec[fps_subsec_cur]).c_str(), 0, 1000, { 100,50 });
      ImGui::TreePop();

      ImGui::LabelText("sector DC", "%i", dc);
      ImGui::LabelText("active sectors", "%i", act);
    }
  }
  ImGui::End();
}

void WindowPerfomance::DtUpdate(float __dt, int __fps, int __dc, int __act)
{
  std::lock_guard<std::mutex> lg(lock);
  fps_sec_timer += __dt;
  fps_subsec_timer += __dt;
  dc = __dc;
  act = __act;

  if (fps_sec_timer >= 1)
  {
    fps_sec[++fps_sec_cur] = static_cast<float>(__fps);
    if (fps_sec_cur == fps_sec.size() - 1)
      fps_sec_cur = 0;
    fps_sec_timer -= 1;

    mems = GetMemoryString(memory_used());
  }

  if (fps_subsec_timer >= 0.1)
  {
    fps_subsec[++fps_subsec_cur] = static_cast<float>(__fps);
    if (fps_subsec_cur == fps_subsec.size() - 1)
      fps_subsec_cur = 0;
    fps_subsec_timer -= 0.1;

    dt = __dt;
  }

  tess_perf.Update(dt);
  gen_perf.Update(dt);
}

void WindowPerfomance::TesselatorDt(float s)
{
  std::lock_guard<std::mutex> lg(lock);
  tess_perf.Set(s);
}

void WindowPerfomance::GeneratorDt(float s)
{
  std::lock_guard<std::mutex> lg(lock);
  gen_perf.Set(s);
}

WindowPerfomance::PerfHelper::PerfHelper(const std::string & s) : label(s)
{
}

void WindowPerfomance::PerfHelper::Draw()
{
  ImGui::LabelText(label.c_str(), "%g ms\t%g mean\t%g max\t%d/sec", tes_dt_shown * 1000, tes_dt_mean * 1000, tes_dt_max * 1000, tes_last);
}

void WindowPerfomance::PerfHelper::Update(float dt)
{
  static float sec{};
  sec += dt;
  if (sec >= 1)
  {
    sec -= 1;

    tes_dt_shown = tes_dt;
    tes_last = tes_cur;
    tes_cur = 0;
  }
}

void WindowPerfomance::PerfHelper::Set(float _tes_dt)
{
  static unsigned count{1};
  tes_dt = _tes_dt;
  tes_cur++;
  tes_dt_max = max(tes_dt_max, _tes_dt);
  count++;
}

float WindowPerfomance::PerfHelper::Get()
{
  return tes_dt;
}
