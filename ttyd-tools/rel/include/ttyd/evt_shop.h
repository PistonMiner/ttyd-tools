#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_shop {

struct EvtShopDisplayLocation
{
	// Where to display the item, only location is relevant
	const char *mapName;

	// Where to stand in order to inspect or buy the item
	const char *hitName;
} __attribute__((__packed__));

static_assert(sizeof(EvtShopDisplayLocation) == 0x8);

struct EvtShopBuyItem
{
	int32_t itemKind;
	int32_t price;
} __attribute__((__packed__));

static_assert(sizeof(EvtShopBuyItem) == 0x8);

struct EvtShopInteractionData
{
	// Name of the shopkeeper NPC
	const char *shopkeeperNpcName;
	
	// Run when talking to the shopkeeper before normal interaction. If LW(0)
	// is -1 when the event completes, return control to Mario and exit. If
	// LW(0) is -100 when the event completes, attach following message windows
	// to the existing message window.
	void *fookEvtCode;

	// Run after buying an item.
	void *buyEvtCode;

	// Message IDs for interactions with shopkeeper. See `gor_shop1_00` and
	// related for an example.
	const char *msgIds[35];
} __attribute__((__packed__));

static_assert(sizeof(EvtShopInteractionData) == 0x98);

struct EvtShopSellPrice
{
	int32_t itemKind;
	int16_t price;
	uint16_t pad_6;
} __attribute__((__packed__));

static_assert(sizeof(EvtShopSellPrice) == 0x8);

extern "C" {

bool evtShopIsActive();

// evt_shop_setup(
//     EvtShopDisplayLocation *displayLocations,
//     EvtShopBuyItem *buyableItems,
//     EvtShopInteractionData *interactionData,
//     EvtShopSellPrice *sellPriceOverrides
// )
// Creates a new item shop. `displayLocations` is a pointer to 6 location infos
// describing where items should be displayed and where you need to stand to
// inspect or buy them. `buyableItems` is a pointer to 6 entries which describe
// items for sale and their price. `interactionData` is a structure describing
// the shopkeeper, special interaction or buy events and messages to when
// interacting with the shop keeper. `sellPriceOverrides`` is a list of special
// item kinds and prices in this shop which don't match the default.
EVT_DECLARE_USER_FUNC(evt_shop_setup, 4)

}

}