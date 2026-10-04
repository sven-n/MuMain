#pragma once

// HUD changes the server reports.
namespace UI::Hud
{
// This player's Gens standing: contribution points, rank, and the points the next rank needs.
void SetGensStanding(int contribution, int ranking, int nextContribution);
}
