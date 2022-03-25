#pragma once

namespace ttyd::mario_party {

extern "C" {

// marioUseParty
// unk_JP_US_EU_32

// Set party member by kind. Returns slot used.
int marioPartyEntry(int partyKindId);

// Set party member by kind with hello animation. Returns slot used.
int marioPartyHello(int partyKindId);

// Remove main party if present. Returns whether there was one.
bool marioPartyGoodbye();

// Remove all party.
void marioPartyKill();

// Get main or extra party kind, preferring main.
int marioGetParty();

// Get slot of main party.
int marioGetPartyId();

// Get slot of extra party.
int marioGetExtraPartyId();

// Add party kind to pouch.
void partyJoin(int partyKindId);

// Remove party kind from pouch.
void partyLeft(int partyKindId);

// Check whether the given party kind has already joined.
bool partyChkJoin(int partyKindId);

// Get current HP for party kind.
int partyGetHp(int partyKindId);

// Get tech level for party kind.
int partyGetTechLv(int partyKindId);

}

}