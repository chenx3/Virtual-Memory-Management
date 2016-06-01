#include <stdlib.h>
#include <stdio.h>
#include "tlb.h"
#include "pagetable.h"
#include "global.h" /* for tlb_size */
#include "statistics.h"

int i;
int j;
int found;
int index;
/*******************************************************************************
 * Looks up an address in the TLB. If no entry is found, calls pagetable_lookup()
 * to get the entry from the page table instead
 *
 * @param vpn The virtual page number to lookup.
 * @param write If the access is a write, this is 1. Otherwise, it is 0.
 * @return The physical frame number of the page we are accessing.
 */
pfn_t tlb_lookup(vpn_t vpn, int write) {
   pfn_t pfn;

   /* 
    * FIX ME : Step 5
    * Note that tlb is an array with memory already allocated and initialized to 0/null
    * meaning that you don't need special cases for a not-full tlb, the valid field
    * will be 0 for both invalid and empty tlb entries, so you can just check that!
    */

   /* 
    * Search the TLB - hit if find valid entry with given VPN 
    * Increment count_tlbhits on hit. 
    */
   index = 0;
   found = 0;
   for (i = 0;i<tlb_size;i++){
     if(tlb[i].valid == 1){
       if(tlb[i].vpn == vpn){
         pfn = tlb[i].pfn;
         count_tlbhits ++;
         found = 1;
         index = i;
         break;
       }
     }
   }
   /* 
    * If it was a miss, call the page table lookup to get the pfn
    * Add current page as TLB entry. Replace any invalid entry first, 
    * then do a clock-sweep to find a victim (entry to be replaced).
    */
   if(found == 0){
     pfn = pagetable_lookup(vpn, write);
   }
   /* 
    *replace invalid entry
   */
   if(found == 0){
     for (j = 0;j<tlb_size;j++){
       if(tlb[j].valid != 1){
         tlb[j].valid = 1;
         tlb[j].vpn = vpn;
         tlb[j].pfn = pfn;
         found = 1;
         index = j;
         break;
       }
     }
   }
   
   /* 
    * Do a clock sweep to find the victim
   */
   if (found == 0){
     for (j = 0;j<tlb_size;j++){
       if(tlb[i].used != 1){
         tlb[j].valid = 1;
         tlb[j].vpn = vpn;
         tlb[j].pfn = pfn;
         index = j;
         break;
       }
       tlb[i].used = 0;
     }
   }
   /* 
    * If all entries are used, the first entry is the victim
   */
   if(found == 0){
     tlb[0].valid = 1;
     tlb[0].vpn = vpn;
     tlb[0].pfn = pfn;
     index = 0;
   }
   /*
    * In all cases perform TLB house keeping. This means marking the found TLB entry as
    * used and if we had a write, dirty. We also need to update the page
    * table entry in memory with the same data.
    */
   tlb[index].used = 1;
   tlb[index].dirty = write;
   current_pagetable[vpn].dirty = write;
   current_pagetable[vpn].used = 1;
   return pfn;
}

