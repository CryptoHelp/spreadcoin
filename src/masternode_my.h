#pragma once

#include "main.h"

class CMasterNode;

class CMasterNodeSecret
{
public:
    CKey privkey;
    CSignature signature;

    CMasterNodeSecret()
    {}

    CMasterNodeSecret(CKey privkey);
};

// Control our masternodes
CMasterNode *MN_SetMy(const COutPoint& outpoint, bool my);
bool MN_StartFromStr(const std::string& secret);
bool MN_Start(const COutPoint& outpoint, const CMasterNodeSecret &secret);
bool MN_Stop(const COutPoint& outpoint);
void MN_MyRemoveOutdated();
void MN_StartFromConfig();

// Process network events
void MN_MyProcessBlock(const CBlockIndex* pBlock);
void MN_MyProcessTx(const CTransaction& tx, int64_t nFees);

extern uint256 g_OurMasternodesXor;
