#ifndef C5223C4D_DEE1_4A62_9875_821D328D3D3F
#define C5223C4D_DEE1_4A62_9875_821D328D3D3F
#ifndef C47D076A_89F6_4EAF_B3A5_03FEA03CC0EC
#define C47D076A_89F6_4EAF_B3A5_03FEA03CC0EC
#ifndef CUSTOM_OS_KERNEL_MM_VMM_H
#define CUSTOM_OS_KERNEL_MM_VMM_H

#include <stdint.h>

int stage8b_vmm_is_page_mapped(uint32_t virtual_page_addr);
int stage8b_vmm_resolve_page_frame(uint32_t virtual_page_addr, uint32_t* out_physical_frame_addr);
int stage8b_vmm_map_page(uint32_t virtual_page_addr, uint32_t physical_frame_addr);
int stage8b_vmm_unmap_page(uint32_t virtual_page_addr);

#endif


#endif /* C47D076A_89F6_4EAF_B3A5_03FEA03CC0EC */


#endif /* C5223C4D_DEE1_4A62_9875_821D328D3D3F */
