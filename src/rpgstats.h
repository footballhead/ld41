/**
 * \file rpgstats.h
 * \brief Helper functions for manipulating player stats.
 */
#pragma once

/**
 * \brief Decrease the player life total by an amount.
 * \param amount UNUSED Value > 0 which will be taken away
 * \returns 0 on success, -1 on failure
 */
int rpgstats_hurt_player(int amount);

int rpgstats_give_bear(void);
