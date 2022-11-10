#include "eco_patch_check.h"

u32 chk_patch_port_dis_flow(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8730E:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

#if 0
u32 chk_patch_l2_ldo(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_aphy_pc(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_flr_lps(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_pcie_vmain(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_otp_power_issue(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_txamsdu_rls_wd_issue(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_cut_amsdu_rls_ple_issue(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_hi_pri_resp_tx(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_port_dis_flow(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8730E:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_dis_resp_chk(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_dis_separation(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_hwamsdu_fa(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_vht_ampdu_max_len(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_haxidma_ind(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}
/*
u32 chk_patch_tbtt_shift_setval(struct mac_ax_adapter *adapter)
{
		switch (adapter->hw_info->chip_id) {
		case MAC_AX_CHIP_ID_8192XB:
				switch (adapter->hw_info->cv) {
				case CAV:
						return PATCH_ENABLE;
				case CBV:
						return PATCH_ENABLE;
				case CCV:
						return PATCH_ENABLE;
				default:
						return PATCH_ENABLE;
				}
				break;
		case MAC_AX_CHIP_ID_8851B:
				switch (adapter->hw_info->cv) {
				case CAV:
						return PATCH_ENABLE;
				case CBV:
						return PATCH_ENABLE;
				default:
						return PATCH_ENABLE;
				}
				break;
		case MAC_AX_CHIP_ID_8852A:
				switch (adapter->hw_info->cv) {
				case CAV:
						return PATCH_ENABLE;
				case CBV:
						return PATCH_ENABLE;
				case CCV:
						return PATCH_ENABLE;
				case CDV:
						return PATCH_ENABLE;
				default:
						return PATCH_ENABLE;
				}
				break;
		case MAC_AX_CHIP_ID_8852B:
				switch (adapter->hw_info->cv) {
				case CAV:
						return PATCH_ENABLE;
				case CBV:
						return PATCH_ENABLE;
				case CCV:
						return PATCH_ENABLE;
				case CDV:
						return PATCH_ENABLE;
				default:
						return PATCH_ENABLE;
				}
				break;
		case MAC_AX_CHIP_ID_8852C:
				switch (adapter->hw_info->cv) {
				case CAV:
						return PATCH_ENABLE;
				case CBV:
						return PATCH_ENABLE;
				default:
						return PATCH_ENABLE;
				}
				break;
		default:
				PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
						 adapter->hw_info->cv);
				return PATCH_ENABLE;
		}
}
*/
u32 chk_patch_tmac_zld_thold(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_reg_sdio(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_pcie_sw_ltr(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_ltssm_card_loss(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_l12_reboot(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_rx_prefetch(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_sic_clkreq(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_pcie_deglitch(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_otp_pwr_drop(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_pcie_err_ind(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_fs_enuf(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_apb_hang(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_pcie_hang(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_cmac_io_fail(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_ENABLE;
		case CDV:
			return PATCH_ENABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}

u32 chk_patch_flash_boot_timing(struct mac_ax_adapter *adapter)
{
	switch (adapter->hw_info->chip_id) {
	case MAC_AX_CHIP_ID_8192XB:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_ENABLE;
		case CBV:
			return PATCH_ENABLE;
		case CCV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8851B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852A:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852B:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		case CCV:
			return PATCH_DISABLE;
		case CDV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	case MAC_AX_CHIP_ID_8852C:
		switch (adapter->hw_info->cv) {
		case CAV:
			return PATCH_DISABLE;
		case CBV:
			return PATCH_DISABLE;
		default:
			return PATCH_ENABLE;
		}
		break;
	default:
		PLTFM_MSG_ALWAYS("Not Support IC version =%x\n",
				 adapter->hw_info->cv);
		return PATCH_ENABLE;
	}
}
#endif
