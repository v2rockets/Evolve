// SimulationOptionsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "evolve.h"
#include "SimulationOptionsDialog.h"


// SimulationOptionsDialog dialog

IMPLEMENT_DYNAMIC(SimulationOptionsDialog, CDialog)
SimulationOptionsDialog::SimulationOptionsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(SimulationOptionsDialog::IDD, pParent)
{
}

SimulationOptionsDialog::~SimulationOptionsDialog()
{
}

void SimulationOptionsDialog::SetKfmo(KFORTH_MUTATE_OPTIONS *kfmo)
{
	ASSERT( kfmo != NULL );

	m_kfmo		= *kfmo;
	modified	= false;
	m_orig_kfmo	= m_kfmo;
}

void SimulationOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_CommaValuePositive(pDX, IDC_SO_MAX_CB, m_kfmo.max_code_blocks);
	DDX_RangeValue(pDX, IDC_SO_MAX_APPLY, m_kfmo.max_apply, 0, MUTATE_MAX_APPLY_LIMIT);
	DDX_MutationRate(pDX, IDC_SO_DUPLICATE, m_kfmo.prob_point_duplicate);
	DDX_MutationRate(pDX, IDC_SO_DELETE, m_kfmo.prob_point_delete);
	DDX_MutationRate(pDX, IDC_SO_INSERT, m_kfmo.prob_point_insert);
	DDX_MutationRate(pDX, IDC_SO_MODIFY, m_kfmo.prob_point_modify);
	DDX_MutationRate(pDX, IDC_SO_B_DUPLICATE, m_kfmo.prob_block_duplicate);
	DDX_MutationRate(pDX, IDC_SO_B_DELETE, m_kfmo.prob_block_delete);
	DDX_MutationRate(pDX, IDC_SO_B_INSERT, m_kfmo.prob_block_insert);
	DDX_MutationRate(pDX, IDC_SO_B_SWAP, m_kfmo.prob_block_swap);
	DDX_MutationRate(pDX, IDC_SO_B_SPLICE, m_kfmo.prob_block_splice);
	DDX_MutationRate(pDX, IDC_SO_B_CROSSOVER, m_kfmo.prob_block_crossover);
	DDX_MutationRate(pDX, IDC_SO_H_SELECT, m_kfmo.prob_home_select);
	DDX_MutationRate(pDX, IDC_SO_H_CROSSOVER, m_kfmo.prob_home_crossover);
	DDX_MutationRate(pDX, IDC_SO_RAY, m_kfmo.prob_cosmic_ray);
	DDX_MutationRate(pDX, IDC_SO_PLOIDY, m_kfmo.prob_ploidy_change);

	if( m_kfmo.max_code_blocks != m_orig_kfmo.max_code_blocks )
		modified = true;

	if( m_kfmo.max_apply != m_orig_kfmo.max_apply )
		modified = true;

	if( m_kfmo.prob_point_duplicate != m_orig_kfmo.prob_point_duplicate )
		modified = true;

	if( m_kfmo.prob_point_delete != m_orig_kfmo.prob_point_delete )
		modified = true;

	if( m_kfmo.prob_point_insert != m_orig_kfmo.prob_point_insert )
		modified = true;

	if( m_kfmo.prob_point_modify != m_orig_kfmo.prob_point_modify )
		modified = true;

	if( m_kfmo.prob_block_duplicate != m_orig_kfmo.prob_block_duplicate )
		modified = true;

	if( m_kfmo.prob_block_delete!= m_orig_kfmo.prob_block_delete )
		modified = true;

	if( m_kfmo.prob_block_insert != m_orig_kfmo.prob_block_insert )
		modified = true;

	if( m_kfmo.prob_block_swap != m_orig_kfmo.prob_block_swap )
		modified = true;

	if( m_kfmo.prob_block_splice != m_orig_kfmo.prob_block_splice )
		modified = true;

	if( m_kfmo.prob_block_crossover!= m_orig_kfmo.prob_block_crossover )
		modified = true;

	if( m_kfmo.prob_home_select != m_orig_kfmo.prob_home_select )
		modified = true;

	if( m_kfmo.prob_home_crossover != m_orig_kfmo.prob_home_crossover )
		modified = true;

	if( m_kfmo.prob_cosmic_ray != m_orig_kfmo.prob_cosmic_ray )
		modified = true;

	if( m_kfmo.prob_ploidy_change != m_orig_kfmo.prob_ploidy_change )
		modified = true;
}

void SimulationOptionsDialog::Help()
{
	CString str;
	CWinApp *app;

	app = AfxGetApp();
	str = app->GetProfileString("help", "path");
	str = str + "\\simulation_options_dialog.html";
	ShellExecute(m_hWnd, "open", str, NULL, NULL, SW_SHOWNORMAL);
}

BEGIN_MESSAGE_MAP(SimulationOptionsDialog, CDialog)
	ON_COMMAND(IDC_SO_HELP, Help)
END_MESSAGE_MAP()


// SimulationOptionsDialog message handlers
