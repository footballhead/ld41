/**
 * \file repeater_lib.h
 * \brief Helper functions for using the repeater service.
 */
#pragma once

/**
 * \brief Send a message to the player,
 * \param message The string message to send (please null terminate!)
 * \returns 0 on success, -1 on failure
 * \remark Bounces off the repeater service.
 */
int print_message_to_player(const char* message);
