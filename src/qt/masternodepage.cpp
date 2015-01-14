#include <map>
#include <QCheckBox>
#include "masternodepage.h"
#include "ui_masternodepage.h"
#include "walletmodel.h"
#include "util.h"
#include "init.h"
#include "masternodes.h"
#include "bitcoinunits.h"

enum EColumn
{
    C_ADDRESS,
    C_AMOUNT,
    C_SCORE,
    C_CONTROL,
    C_OUTPUT,

    C_COUNT,
};

MasternodeCheckbox::MasternodeCheckbox(CKeyID keyid_, const COutPoint &outpoint_)
    : keyid(keyid_), outpoint(outpoint_)
{
    connect(this, SIGNAL(stateChanged(int)), this, SLOT(updateState(int)));
}

void MasternodeCheckbox::updateState(int newState)
{
    emit switchMasternode(keyid, outpoint, newState == Qt::Checked);
}

MasternodePage::MasternodePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MasternodePage),
    model(NULL)
{
    ui->setupUi(this);

    ui->tableWidget->setColumnCount((int)C_COUNT);

    ui->tableWidget->setHorizontalHeaderItem((int)C_ADDRESS, new QTableWidgetItem(tr("Address")));
    ui->tableWidget->setHorizontalHeaderItem((int)C_AMOUNT,  new QTableWidgetItem(tr("Amount")));
    ui->tableWidget->setHorizontalHeaderItem((int)C_OUTPUT,  new QTableWidgetItem(tr("Output")));
    ui->tableWidget->setHorizontalHeaderItem((int)C_CONTROL, new QTableWidgetItem(tr("Control")));
    ui->tableWidget->setHorizontalHeaderItem((int)C_SCORE,   new QTableWidgetItem(tr("Score")));
}

MasternodePage::~MasternodePage()
{
    delete ui;
}

void MasternodePage::setModel(WalletModel *model)
{
    this->model = model;
}

void MasternodePage::showEvent(QShowEvent *)
{
    updateMasternodes();
}

void MasternodePage::updateMasternodes()
{
    LOCK(cs_main);

    if (IsInitialBlockDownload())
        return;

    std::map<QString, std::vector<COutput> > mapCoins;
    model->listCoins(mapCoins);

    BOOST_FOREACH(PAIRTYPE(QString, std::vector<COutput>) coins, mapCoins)
    {
        BOOST_FOREACH(const COutput& out, coins.second)
        {
            COutPoint outpoint(out.tx->GetHash(), out.i);
            MN_SetMy(outpoint, true);
        }
    }

    QTableWidget* pTable = ui->tableWidget;
    pTable->setRowCount(0);

    for (const std::pair<COutPoint, CMasterNode>& pair : g_MasterNodes)
    {
        const CMasterNode& mn = pair.second;

        CBitcoinAddress address;
        address.Set(mn.keyid);

        int iRow = pTable->rowCount();
        pTable->setRowCount(iRow + 1);
        pTable->setItem(iRow, (int)C_ADDRESS, new QTableWidgetItem(address.ToString().c_str()));
        pTable->setItem(iRow, (int)C_AMOUNT, new QTableWidgetItem(BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, mn.amount)));
        pTable->setItem(iRow, (int)C_OUTPUT, new QTableWidgetItem(QString("%1:%2").arg(mn.outpoint.hash.ToString().c_str()).arg(mn.outpoint.n)));
        pTable->setItem(iRow, (int)C_SCORE, new QTableWidgetItem(QString("%1").arg(mn.GetScore())));

        if (mn.my)
        {
            MasternodeCheckbox* pButton = new MasternodeCheckbox(mn.keyid, mn.outpoint);
            pButton->setChecked(mn.privkey.IsValid());
            connect(pButton, SIGNAL(switchMasternode(const CKeyID&, const COutPoint&, bool)), this, SLOT(switchMasternode(const CKeyID&, const COutPoint&, bool)));
            pTable->setCellWidget(iRow, (int)C_CONTROL, pButton);
        }
    }
}

void MasternodePage::switchMasternode(const CKeyID &keyid, const COutPoint &outpoint, bool state)
{
    LOCK(cs_main);

    if (!state)
        MN_Stop(outpoint);
    else
    {
        CKey key;
        WalletModel::UnlockContext context(model->requestUnlock());
        pwalletMain->GetKey(keyid, key);
        MN_Start(outpoint, key);
    }
}
