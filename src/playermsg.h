/**
 * \file playermsg.h
 * \brief Helper functions for showing messages to the player.
 */
#pragma once

/**
 * \brief Send a message to the player,
 * \param message The string message to send (please null terminate!)
 * \returns 0 on success, -1 on failure
 */
int print_message_to_player(const char* message);
