/*****************************************************************************/
/*  LibreDWG - free implementation of the DWG file format                    */
/*                                                                           */
/*  Copyright (C) 2009 Free Software Foundation, Inc.                        */
/*                                                                           */
/*  This library is free software, licensed under the terms of the GNU       */
/*  General Public License as published by the Free Software Foundation,     */
/*  either version 3 of the License, or (at your option) any later version.  */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program.  If not, see <http://www.gnu.org/licenses/>.    */
/*****************************************************************************/

/**
 *     \file       classes.c
 *     \brief      Class section functions
 *     \author     written by Felipe Castro
 *     \author     modified by Felipe Corrêa da Silva Sances
 *     \author     modified by Rodrigo Rodrigues da Silva
 *     \author     modified by Till Heuschmann
 *     \version    
 *     \copyright  GNU General Public License (version 3 or later)
 */

#include "classes.h"
#include "compress_decompress.h"
#include "logging.h"

/** R13_R15 Class Section */
void
read_R13_R15_section_classes(Bit_Chain *dat, Dwg_Data *dwg)
{
  uint8_t ckr, ckr2;
  uint32_t size, lasta, pvz;

  LOG_INFO("\nCLASS: %8X     \n", (unsigned int)dwg->header.section[1].address)
  LOG_INFO("CLASS (end): %8X \n", (unsigned int)(dwg->header.section[1].address
                                   + dwg->header.section[1].size))

  dat->byte = dwg->header.section[1].address + 16;
  dat->bit  = 0;
  size  = bit_read_RL(dat);
  lasta = dat->byte + size;
  LOG_TRACE("Length: %x \n", size);

  //read the classes
  dwg->dwg_ot_layout = 0;
  dwg->num_classes = 0;

  do
    {
      unsigned int idc;

      idc = dwg->num_classes;

      if (idc == 0)
        dwg->dwg_class = (Dwg_Class *) malloc(sizeof(Dwg_Class));
      else
        dwg->dwg_class = (Dwg_Class *) realloc(dwg->dwg_class, (idc + 1)
                          * sizeof(Dwg_Class));

      dwg->dwg_class[idc].number        = bit_read_BS(dat);
      dwg->dwg_class[idc].version       = bit_read_BS(dat);
      dwg->dwg_class[idc].appname       = bit_read_T(dat);
      dwg->dwg_class[idc].cppname       = bit_read_T(dat);
      dwg->dwg_class[idc].dxfname       = bit_read_T(dat);
      dwg->dwg_class[idc].wasazombie    = bit_read_B(dat);
      dwg->dwg_class[idc].item_class_id = bit_read_BS(dat);

      if (strcmp((const char *)dwg->dwg_class[idc].dxfname, "LAYOUT") == 0)
        dwg->dwg_ot_layout = dwg->dwg_class[idc].number;

      dwg->num_classes++;
      /*
       * if (dwg->num_classes > 100)
       * {
       *   fprintf(stderr, "number of classes is greater than 100. TODO:
       *                   Why should we stop here?\n");
       *   break; //TODO: Why?!
       * }
       */
    }
  while (dat->byte < (lasta - 1));

  // Check CRC-on
  dat->byte = dwg->header.section[1].address + dwg->header.section[1].size
              - 18;
  dat->bit  = 0;

  ckr  = bit_read_RS(dat);
  ckr2 = bit_ckr8(0xc0c1, dat->chain + dwg->header.section[1].address + 16,
                  dwg->header.section[1].size - 34);

  if (ckr != ckr2)
    {
      printf("Section %d crc todo ckr: %x ckr2:%x \n",
              dwg->header.section[1].number, ckr, ckr2);
      return -1;
    }
  dat->byte += 16;
  pvz = bit_read_RL(dat); // Unknown bitlong inter class and object

  LOG_TRACE("Address: %lu / Content: 0x%#x \n", dat->byte - 4, pvz)
  LOG_INFO("Number of classes read: %u \n", dwg->num_classes)
}

/** R2004 Class Section */
void
read_R2004_section_classes(Bit_Chain *dat, Dwg_Data *dwg)
{
  Bit_Chain sec_dat;
  //uint32_t dwg_version, maint_version, max_num, num_objects, size, unknown;
  uint32_t size;

  if (read_R2004_compressed_section(dat, dwg, &sec_dat, SECTION_CLASSES) != 0)
    return;

  if (bit_search_sentinel(&sec_dat, dwg_sentinel(DWG_SENTINEL_CLASS_BEGIN)))
    {
      //char c;

      size    = bit_read_RL(&sec_dat);     // size of class data area
      //max_num = bit_read_BS(&sec_dat);  // Maxiumum class number
      //c       = bit_read_RC(&sec_dat);        // 0x00
      //c       = bit_read_RC(&sec_dat);        // 0x00
      //c       = bit_read_B(&sec_dat);         // 1
      dwg->dwg_ot_layout = 0;
      dwg->num_classes   = 0;

      do
        {
          unsigned int idc;

          idc = dwg->num_classes;

          if (idc == 0)
            dwg->dwg_class = (Dwg_Class *) malloc(sizeof (Dwg_Class));
          else
            dwg->dwg_class = (Dwg_Class *) realloc(dwg->dwg_class, (idc + 1)
                              * sizeof (Dwg_Class));

          dwg->dwg_class[idc].number        = bit_read_BS(&sec_dat);
          dwg->dwg_class[idc].version       = bit_read_BS(&sec_dat);
          dwg->dwg_class[idc].appname       = bit_read_T(&sec_dat);
          dwg->dwg_class[idc].cppname       = bit_read_T(&sec_dat);
          dwg->dwg_class[idc].dxfname       = bit_read_T(&sec_dat);
          dwg->dwg_class[idc].wasazombie    = bit_read_B(&sec_dat);
          dwg->dwg_class[idc].item_class_id = bit_read_BS(&sec_dat);

          //num_objects   = bit_read_BL(&sec_dat);  // DXF 91
          //dwg_version   = bit_read_BS(&sec_dat);  // Dwg Version
          //maint_version = bit_read_BS(&sec_dat);  // Maintenance release version.
          //unknown       = bit_read_BL(&sec_dat);  // Unknown (normally 0L)
          //unknown       = bit_read_BL(&sec_dat);  // Unknown (normally 0L)

          LOG_TRACE("\n")
          LOG_TRACE("Number: %d \n",           dwg->dwg_class[idc].number)
          LOG_TRACE("Version: %x \n",          dwg->dwg_class[idc].version)
          LOG_TRACE("Application name: %s \n", dwg->dwg_class[idc].appname)
          LOG_TRACE("C++ class name: %s \n",   dwg->dwg_class[idc].cppname)
          LOG_TRACE("DXF record name: %s \n",  dwg->dwg_class[idc].dxfname)

          if (strcmp((const char *)dwg->dwg_class[idc].dxfname, "LAYOUT") == 0)
            dwg->dwg_ot_layout = dwg->dwg_class[idc].number;

          dwg->num_classes++;
        }
      while (sec_dat.byte < (size - 1));
    }
  /* Check CRC on */
  uint32_t pvz, ckr, ckr2;
  
  sec_dat.byte = dwg->header.section[1].address + dwg->header.section[1].size
                 - 18;
  sec_dat.bit  = 0;
 
  ckr  = bit_read_RL(dat);
  ckr2 = bit_ckr32(0xc0c1, dat->chain + dwg->header.section[1].address + 16,
                   dwg->header.section[1].size - 34);
 
  if (ckr != ckr2)
    {
      printf("Section %d crc todo ckr:%x ckr2:%x \n\n",
              dwg->header.section[1].number, ckr, ckr2);
      return -1;
    }
  sec_dat.byte += 16;
  pvz = bit_read_RL(dat); // Unknown bitlong inter class and object

  LOG_TRACE("Address: %lu / Content: 0x%#x \n", sec_dat.byte - 4, pvz)
  LOG_INFO("Number of classes read: %u\n", dwg->num_classes)
  free(sec_dat.chain);
}

/** R2007 Class Section */
void
read_R2007_section_classes(Bit_Chain *dat, Dwg_Data *dwg, 
                           r2007_section *sections_map, r2007_page *pages_map)
{
  Bit_Chain sec_dat;
  
  if (read_data_section(&sec_dat, dat, sections_map, pages_map, 0x3f54045f) != 0)
    return;  

  if (bit_search_sentinel(&sec_dat, dwg_sentinel(DWG_SENTINEL_CLASS_BEGIN)))
    {
      Bit_Chain sstream;
      char unknown;
      //uint32_t dwg_version, maint_version, num_objects, unknown1, unknown2;
      uint32_t max_num, num_bits, size;

      size     = bit_read_RL(&sec_dat);  // size of class data area
      num_bits = bit_read_RL(&sec_dat);  // size in bits
      max_num  = bit_read_BL(&sec_dat);  // Maxiumum class number
      //unknown  = bit_read_B(&sec_dat);

      string_stream_init(&sstream, &sec_dat, num_bits, 0);
      dwg->dwg_ot_layout = 0;
      dwg->num_classes   = 0;
    
      do
        {
          unsigned int idc;

          idc = dwg->num_classes;

          if (idc == 0)
            dwg->dwg_class = (Dwg_Class *) malloc(sizeof(Dwg_Class));
          else
            dwg->dwg_class = (Dwg_Class *) realloc(dwg->dwg_class, (idc + 1)
                              * sizeof(Dwg_Class));

          dwg->dwg_class[idc].number        = bit_read_BS(&sec_dat);
          dwg->dwg_class[idc].version       = bit_read_BS(&sec_dat);
          dwg->dwg_class[idc].wasazombie    = bit_read_B(&sec_dat);
          dwg->dwg_class[idc].item_class_id = bit_read_BS(&sec_dat);  // 0x1F2 or 0x1F3
          dwg->dwg_class[idc].appname       = (char*)bit_read_TU(&sstream);
          dwg->dwg_class[idc].cppname       = (char*)bit_read_TU(&sstream);
          dwg->dwg_class[idc].dxfname       = (char*)bit_read_TU(&sstream);
        
          //num_objects   = bit_read_BL(&sec_dat);  // DXF 91
          //dwg_version   = bit_read_BL(&sec_dat);  // Dwg Version
          //maint_version = bit_read_BL(&sec_dat);  // Maintenance release version
          //unknown1      = bit_read_BL(&sec_dat);  // Unknown (normally 0L)
          //unknown2      = bit_read_BL(&sec_dat);  // Unknown (normally 0L)

          dwg->num_classes++;
        
          if (dwg->dwg_class[idc].number == max_num)
            break;
        } 
      while (sec_dat.byte < (size - 1));
    }
  free(sec_dat.chain);
}
