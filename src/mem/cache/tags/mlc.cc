/*
 * Copyright (c) 2012-2013 ARM Limited
 * All rights reserved.
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2003-2005,2014 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Erik Hallnor
 */

/**
 * @file
 * Definitions of a MLC tag store.
 */

#include "mem/cache/tags/mlc.hh"
#include <queue>
#include "debug/CacheRepl.hh"
#include "mem/cache/base.hh"

MLC::MLC(const Params *p)
    : BaseSetAssoc(p)
{
	encodingSize = p->encodingSize;
	//shiftSize = p->shiftSize;
	flipSize = p->flipSize;
	thres = p->thres;
	loc_weight = p->loc_weight;
	diverse_weight = p->diverse_weight;
	shiftSize = 64;
	options = p->options;
	UUthres = p->UUthres;
	enc_correction = p->enc_correction;
	tie_weight = p->tie_weight;
	enc_remap_op = p->enc_remap_op;
	/*encodingSize = 8;
	shiftSize = 64;
	flipSize = 8;
	thres = 47;
	loc_weight = 8;*/
	std::cout<<"MLC "<< "enc_remap_op "<< enc_remap_op << " " <<"Tie-breaker weight: "<< tie_weight<<" "<< diverse_weight<<" "<<loc_weight<<" thres"<< thres<<" "<< encodingSize <<" " <<flipSize <<" options = " <<options << "UU threshold = " << UUthres << "enc_correction " << enc_correction <<std::endl;
	//std::cout<<"sec "<< diverse_weight<<" "<<loc_weight<<" thres"<< thres<<" "<< encodingSize <<" " <<flipSize <<" options " <<options <<std::endl;
	//std::cout<<"sector "<< secSize <<" entry_size"<<entrySize<<"shift " <<secShift<<std::endl;
}

CacheBlk*
MLC::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id)
{
    CacheBlk *blk = BaseSetAssoc::accessBlock(addr, is_secure, lat, master_id);

    if (blk != nullptr) {
        // move this block to the front of the tree
        sets[blk->set].moveToFront(blk);
        DPRINTF(CacheRepl, "set %x: moving blk %x (%s) to front\n",
                blk->set, regenerateBlkAddr(blk->tag, blk->set),
                is_secure ? "s" : "ns");
    }

    return blk;
}
std::vector<int>
MLC::lineCompare( const Byte* ablock, const Byte* bblock, int size, int shiftSize, int flipSize, int flipBits){ // flip and write :: count all bits and flip all bits
		int mask = 1;
	int fs = flipSize;
	if( flipSize == 0 ) fs = size+1; // no flip
	else
		mask = 1<<( (size/fs) -1 );
	std::unordered_map<int, int> normal_cnt;
	std::unordered_map<int, int> rev_cnt;
	std::vector<int> ret(5, 512);
	ret[4] = 0;
	//int minimal_bits = 512;
	//for (int i =0; i<size; i += shiftSize){
		Byte from, to;
		std::vector<int> res(5,0);// ZT, ST, HT, TT
		//int total_bits = 0;
		//int flipbits = 0;
		bool ifFlip = flipBits & mask;
		int cnt01 = 0;
		int cnt10 = 0;
		for(int j = 0; j < size; j++){
			from = ablock[j]; // from block
			if(ifFlip) from = ~from;
			to = bblock[j]; // to block


			for(int k = 0; k<8; k += 2)  {
				int label = (((from >> k) & 3)*10) + ((to >> k) & 3); // 3 for 0b11
				int rev_label = (((from >> k) & 3)*10) + 3 - ((to >> k) & 3);
				if(label  == 30 or label == 3 or label == 12 or label == 21) cnt10 += 2; // two bits change
				else if(label == 31 or label == 13 or label == 20 or label == 2 or label == 1 or label == 10 or label == 32 or label == 23 ) cnt01 ++; // one bit change
				//std::cout<<"label "<< label << " rev " << rev_label<<std::endl;
				normal_cnt[label] += 1;
				rev_cnt[rev_label] += 1;// 3 is the mask 0x11. count every 2 bits in a byte

			}

			if(fs <= size && (j+1)% fs == 0 ){ // to decide if flip
				if(cnt01 + cnt10 <= fs*4){ // no flip this time

					res[4] = res[4]<<1 ;
					//bool ifSwap = false;
					/*if( normal_cnt[1] + normal_cnt[11] + normal_cnt[22] + normal_cnt[32] < normal_cnt[2] + normal_cnt[12] +normal_cnt[21] +normal_cnt[31]){
						//ifSwap = true;
						std::swap(normal_cnt[1], normal_cnt[2]);
						std::swap(normal_cnt[11], normal_cnt[12]);
						std::swap(normal_cnt[21], normal_cnt[22]);
						std::swap(normal_cnt[31], normal_cnt[32]);
					}*/
					for(auto it : normal_cnt){
						if(it.first == 0 or it.first == 33 or it.first == 22 or it.first == 11 )//zt
							res[0] += it.second;
						else if(it.first == 1 or it.first == 10 or it.first == 32 or it.first == 23)//st
							res[1] += it.second;
						else if(it.first == 3 or it.first == 13 or it.first == 20 or it.first == 30) //ht
							res[2] += it.second;
						else
							res[3] += it.second; //tt
					}
				}else{
					res[4] = (res[4]<<1) + 1;
					/*if( rev_cnt[1] + rev_cnt[11] + rev_cnt[22] + rev_cnt[32] < rev_cnt[2] + rev_cnt[12] + rev_cnt[21] + rev_cnt[31]){
						//ifSwap = true;
						std::swap(rev_cnt[1], rev_cnt[2]);
						std::swap(rev_cnt[11], rev_cnt[12]);
						std::swap(rev_cnt[21], rev_cnt[22]);
						std::swap(rev_cnt[31], rev_cnt[32]);
					}*/
					for(auto it : rev_cnt){
						if(it.first == 0 or it.first == 33 or it.first == 22 or it.first == 11 )//zt
							res[0] += it.second;
						else if(it.first == 1 or it.first == 10 or it.first == 32 or it.first == 23)//st
							res[1] += it.second;
						else if(it.first == 3 or it.first == 13 or it.first == 20 or it.first == 30) //ht
							res[2] += it.second;
						else
							res[3] += it.second;
					}
				}
				mask = mask >> 1;
				ifFlip = flipBits & mask;
				normal_cnt.clear();
				rev_cnt.clear();
				cnt01 = 0;
				cnt10 = 0;
			}
		}
		if( fs > size){// no filp at all
				for(auto it : normal_cnt){
						if(it.first == 0 or it.first == 33 or it.first == 22 or it.first == 11 )//zt
							res[0] += it.second;
						else if(it.first == 1 or it.first == 10 or it.first == 32 or it.first == 23)//st
							res[1] += it.second;
						else if(it.first == 3 or it.first == 13 or it.first == 20 or it.first == 30) //ht
							res[2] += it.second;
						else
							res[3] += it.second;
						}
						res[4] = 0;
		}

		//if(res[3] + res[2] < ret[3] + ret[2])
		ret = res;
	//}
	assert(ret[3] + ret[2] + ret[0] + ret[1] == 256);
	return ret;
}

int MLC::generate_encoding(const Byte* blk_data, int blk_size, const int encodingSize, int thres) {
    int hibit = 0;
    int encoding = 0;
    for(int i = 0; i < blk_size; i++) {
        for(int j = 0; j < 8; j+=2) {
            if(blk_data[i] & (1<<j))
                hibit++;
        }
        if(((i+1)%encodingSize) == 0) {
            /* One chunk processed */
            encoding = (encoding << 1) | (hibit > thres or hibit < (encodingSize*4 - thres));
            encoding = 0; //reset to start new chunk
        }
    }
    return encoding;
}

int MLC::generate_exact_encoding(const Byte* blk_data, int blk_size, const int encodingSize, int thres, int flipbits) {
    int enc = generate_encoding(blk_data, blk_size, encodingSize, thres);
    int real_enc = 0;
    int numSeg = (blk_size/encodingSize);
    if(enc_correction) {
        /* Traverse flipbits and victim encoding at the same time and apply correction */
        for(int i = (numSeg-1); i >= 0; i--) {
            if((((enc >> i) & 1) == 0) &&
               (((flipbits >> 2*i) & 3) != 0)) //This mandates that we must have a no-remap option with encoding correction
                /* Got a "Diverse" that is actually "U" after remap */
                real_enc = (real_enc << 1) | 1;
            else
                real_enc = (real_enc << 1) | ((enc >> i) & 1);
        }
    }
    else
        real_enc = enc;
    return real_enc;
}

int MLC::getNumD(const Byte* blk_data, int blk_size, const int encodingSize, int thres) {
    int encoding = generate_encoding(blk_data, blk_size, encodingSize, thres);
    int numU = 0;
	for(int v = encoding; v > 0; v &= (v-1))
		numU++;
    return ((blk_size/encodingSize) - numU);
}


int MLC::encodingCompare_exact(const Byte* ablock, const Byte* bblock, int size, int victim_flipbits, const int encodingSize, int thres) {
	double total_diff = 0;
	int minimal_diff = 2000000; // Not sure if we need minimal diff but retaining for compatibility, if any
	if((thres >= 0) && encodingSize > 0){
        int numSeg = (size/encodingSize) + 1;
        int victim_encoding = generate_encoding(ablock, size, encodingSize, thres);
        int encoding = generate_encoding(bblock, size, encodingSize, thres);
        int vreal_enc = 0;
        if(enc_correction) {
            /* Traverse flipbits and victim encoding at the same time and apply correction */
            for(int i = (numSeg-2); i >= 0; i--) {
                if((((victim_encoding >> i) & 1) == 0) &&
                   (((victim_flipbits >> 2*i) & 3) != 0))
                    /* Got a "Diverse" that is actually "U" after remap */
                    vreal_enc = (vreal_enc << 1) | 1;
                else
                    vreal_enc = (vreal_enc << 1) | ((victim_encoding >> i) & 1);
            }
        }
        else
            vreal_enc = victim_encoding; //don't apply any correction this may have selection inefficiency but can give energy gains

	    /* Compare each bit of encoding generated for to block and corrected encoding for victim block */
	    for(int i = (numSeg-2); i >= 0; i--) { //decrement start by 2 as we already incremented numSeg by 1
            if(((encoding >> i)&1) != ((vreal_enc >> i)&1))
                total_diff += numSeg;
            else if(((encoding >> i)&1) == 0)
                total_diff++;
	    }
	}
	return (minimal_diff = (total_diff < minimal_diff)? total_diff : minimal_diff);
}

int MLC::encodingCompare_2bit(const Byte* ablock, const Byte* bblock, int size, int shiftSize, int flipSize, int thres, int encodingSize, int zeroWeight){
	if(flipSize == 0 ) flipSize = size+1;
	int minimal_diff = 2000000;
	//for (int i =0; i<size; i += shiftSize) {
		//Byte temp;
	int numSeg = (size/encodingSize) + 1;
	curr_blk_enc_trans = 0; //init the encoding state change from A to B as all 0s
	double total_diff = 0;
		int abits = 0, bbits = 0;
	if(thres >= 0){	// count the #01 and 00
		for(int j = 0; j<size; j++){
				for(int k = 1; k<8; k += 2)  {
					if (ablock[j] & (1<<k) ) abits++; // count every 2nd bit in a byte
					if (bblock[j] & (1<<k) ) bbits++;
				}
				if( (j+1)%encodingSize == 0 ) {
					if(abits > thres or abits < (encodingSize*4 - thres) ) abits = 1; // 1 for domination
					else abits = 0;
					if(bbits > thres or bbits < (encodingSize*4 - thres) ) bbits = 1;
					else bbits = 0;
					curr_blk_enc_trans = (curr_blk_enc_trans << 2) | (abits << 1) | bbits;
					if(abits != bbits) total_diff += numSeg;
					else if( abits == 0) total_diff += 1;
					abits = 0;
					bbits = 0;
				}
		}
		//total_bits += flipbits;  // if no flip..
		if(total_diff < minimal_diff) minimal_diff = total_diff;
	}else{ // count the #11 and 00
		int thr = - thres; // negative the thres
		for(int j = 0; j<size; j++){
				for(int k = 1; k<8; k += 2)  {
					if ( (ablock[j] & (1<<k)) == (ablock[j] & (1<<(k-1)))) abits++; // count every 2nd bit in a byte
					if ( (bblock[j] & (1<<k)) == (bblock[j] & (1<<(k-1)))) bbits++;
				}
				if( (j+1)%encodingSize == 0 ) {
					if(abits > thr ) abits = 1; // 1 for same dominate
					else abits = 0;
					if(bbits > thr ) bbits = 1;
					else bbits = 0;
					if(abits != bbits) total_diff += numSeg;
					else if( abits == 0 ) total_diff += 1;
					abits = 0;
					bbits = 0;
				}
		}
		//total_bits += flipbits;  // if no flip..
		if(total_diff < minimal_diff) minimal_diff = total_diff;
	}
	//}
	//assert(minimal_diff <= size/encodingSize);
	return minimal_diff;
}

std::vector<int> MLC::lineCompare_2bit_mapping( const Byte* ablock, const Byte* bblock, int size, int shiftSize, int flipSize, int flipBits){
	int mask = 0;
	int fs = flipSize; // the size the chunk , if flipsize iis 0 means no flip
	if( flipSize == 0 ) fs = size+1; // no flip
	else
		mask = 2*(size/fs) - 2;
	std::unordered_map<int, int> normal_cnt;
	//std::unordered_map<int, int> rev_cnt;
	std::vector<int> ret(5, 512);
	//int mask1=0x
	ret[4] = 0;
	//int minimal_bits = 512;
	//for (int i =0; i<size; i += shiftSize){
		Byte from, to;
		std::vector<int> res(5,0);// ZT, ST, HT, TT & I use the the res[4] to return the new flip bits string
		//int total_bits = 0;
		//int flipbits = 0;
		int ifFlip = (flipBits >>mask) &3; // ifFlip records the flip option information, because in the block I stored the real content, we need remmaped it back to cell content.
		//int cnt01 = 0;
		//int cnt10 = 0;
		for(int j = 0; j < size; j++){ // for each byte
			from = ablock[j]; // from block
			if(ifFlip == 3 ) from = ~from; // this is flip all
			to = bblock[j]; // to is the new block


			for(int k = 0; k<8; k += 2)  { // for each 2-bit inside a byte
				int label_fr = (((from >> k) & 3));
				int label_to =  ((to >> k) & 3); // 3 for 0b11

				if(ifFlip == 1 && (label_fr == 0 or label_fr == 3) ) // 00 11 swap
					label_fr = 3 - label_fr;
				else if(ifFlip == 2 && (label_fr == 1 or label_fr == 2) ) //01 10 swap
					label_fr = 3- label_fr;
				int label = label_fr *10 + label_to; // so we use label to indicate the 2-bit old cell content and 2-bit new real content
				//int rev_label = (((from >> k) & 3)*10) + 3 - ((to >> k) & 3);
				//if(label  == 30 or label == 31 or label == 20 or label == 21) cnt10++; // significant bits change from 1 to 0
				//else if(label == 3 or label == 13 or label == 2 or label == 12 ) cnt01 ++; // significant bits change from 0 to 1
				//std::cout<<"label "<< label << " rev " << rev_label<<std::endl;
				normal_cnt[label] += 1;
				//rev_cnt[rev_label] += 1;// 3 is the mask 0x11. count every 2 bits in a byte

			}

			if(fs <= size && (j+1)% fs == 0 ){ // when the byte is multiple of chunk size , need to decide if we flip this chunk
			//Flip decision is made by the count. Rakesh you need change the decision logic here
				if( normal_cnt[0] + normal_cnt[10] + normal_cnt[23] + normal_cnt[33] >= normal_cnt[3] + normal_cnt[13] +normal_cnt[20] +normal_cnt[30]){ // no 00/11 change

					res[4] = res[4]<<1 ; // 0* ->>I recorded the flip options, which will be return

					//bool ifSwap = false;
					res[4] = res[4]<<1 ; // 00 ->>I recorded the flip options, which will be return
					if( normal_cnt[1] + normal_cnt[11] + normal_cnt[22] + normal_cnt[32] < normal_cnt[2] + normal_cnt[12] +normal_cnt[21] +normal_cnt[31]){
						//ifSwap = true;
						res[4] += 1; // 01 ->>I recorded the flip options, which will be return
						std::swap(normal_cnt[1], normal_cnt[2]); // use swap to prepare  the value for transistions counting
						std::swap(normal_cnt[11], normal_cnt[12]); // do all 01/10 exchange
						std::swap(normal_cnt[21], normal_cnt[22]);
						std::swap(normal_cnt[31], normal_cnt[32]);
					}
					for(auto it : normal_cnt){ // count transistions
						if(it.first == 0 or it.first == 33 or it.first == 22 or it.first == 11 )//zt
							res[0] += it.second;
						else if(it.first == 1 or it.first == 10 or it.first == 32 or it.first == 23)//st
							res[1] += it.second;
						else if(it.first == 3 or it.first == 13 or it.first == 20 or it.first == 30) //ht
							res[2] += it.second;
						else
							res[3] += it.second; //tt
					}
				}else{ // do 00/11 exchange first
					res[4] = (res[4]<<1) or 1; // 1* ->>I recorded the flip options, which will be return
					std::swap(normal_cnt[0], normal_cnt[3]); // do all 11/00 exhange
					std::swap(normal_cnt[10], normal_cnt[13]);
					std::swap(normal_cnt[23], normal_cnt[20]);
					std::swap(normal_cnt[33], normal_cnt[30]);
					res[4] = (res[4]<<1); // 10 ->>I recorded the flip options, which will be return
					if( normal_cnt[1] + normal_cnt[11] + normal_cnt[22] + normal_cnt[32] < normal_cnt[2] + normal_cnt[12] + normal_cnt[21] + normal_cnt[31]){
						//ifSwap = true;
						res[4] += 1; //   11->>I recorded the flip options, which will be return
						std::swap(normal_cnt[1], normal_cnt[2]);
						std::swap(normal_cnt[11], normal_cnt[12]);
						std::swap(normal_cnt[21], normal_cnt[22]);
						std::swap(normal_cnt[31], normal_cnt[32]);
					}
					for(auto it : normal_cnt){
						if(it.first == 0 or it.first == 33 or it.first == 22 or it.first == 11 )//zt
							res[0] += it.second;
						else if(it.first == 1 or it.first == 10 or it.first == 32 or it.first == 23)//st
							res[1] += it.second;
						else if(it.first == 3 or it.first == 13 or it.first == 20 or it.first == 30) //ht
							res[2] += it.second;
						else
							res[3] += it.second;
					}
				}
				mask = mask -2;
				ifFlip = (flipBits >> mask) & 3; // update the ifPlip for next chunk
				normal_cnt.clear();
				//rev_cnt.clear();
				//cnt01 = 0;
				//cnt10 = 0;
			}
		}
		if( fs > size){// no filp at all
				for(auto it : normal_cnt){
						if(it.first == 0 or it.first == 33 or it.first == 22 or it.first == 11 )//zt
							res[0] += it.second;
						else if(it.first == 1 or it.first == 10 or it.first == 32 or it.first == 23)//st
							res[1] += it.second;
						else if(it.first == 3 or it.first == 13 or it.first == 20 or it.first == 30) //ht
							res[2] += it.second;
						else
							res[3] += it.second;
						}
						res[4] = 0;
		}

		//if(res[3] + res[2] < ret[3] + ret[2])
		ret = res;
	//}
	assert(ret[3] + ret[2] + ret[0] + ret[1] == 256);
	return ret;
}

enum EStateTrans {
    EZT,
    EST,
    EHT,
    ETT
};

const double energy_val[4] = {/*ZT*/ 0, /*ST*/ 1.92, /*HT*/ 3.192, /*TT*/ 3.192+1.92};

/* Stateful mapping schemes */
/** NO REMAP    (00)    :        L00->R00, L01->R01, L10->R10, L11->R11 **/
/** FLIP ALL    (01)    :        L00->R11, L01->R10, L10->R01, L11->R00 **/
/** FLIP HI     (10)    :        L00->R10, L01->R11, L10->R00, L11->R01 **/
/** MIXED FLIP  (11)    :        L00->R10, L01->R11, L10->R01, L11->R00 **/

enum ERemapSchemes {
    ENOREMAP,
    EFLIPALL,
    EFLIPHI,
    EFLIPMIXED,
    EREMAPINVALID
    };

int stateful_remaps[4][4] = {   {0, 1, 2, 3},   /*  NO REMAP    */
                                {3, 2, 1, 0},   /*  FLIP ALL    */
                                {2, 3, 0, 1},   /*  FLIP HI     */
                                {2, 3, 1, 0}    /*  MIXED FLIP  */    };


int remap_energy_tbl[4][4][4] = {
    /*  NO REMAP    */ {
        {00, 11, 22, 33},   /*  ZT  */
        {10, 01, 32, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  FLIP ALL    */ {
        {03, 12, 21, 30},   /*  ZT  */
        {13, 02, 31, 20},   /*  ST  */
        {00, 10, 23, 33},   /*  HT  */
        {01, 11, 22, 32}    /*  TT  */
        },

    /*  FLIP HI     */ {
        {02, 13, 20, 31},   /*  ZT  */
        {12, 03, 30, 21},   /*  ST  */
        {22, 32, 01, 11},   /*  HT  */
        {23, 33, 00, 10}    /*  TT  */
        },

    /*  MIXED FLIP  */ {
        {20, 31, 12, 03},   /*  ZT  */
        {30, 21, 13, 02},   /*  ST  */
        {01, 11, 23, 33},   /*  HT  */
        {00, 10, 22, 32}    /*  TT  */
        }
};


/* State transforming remaps */
/** NO REMAP     (00)    :        L00->R00, L01->R01, L10->R10, L11->R11 **/
/** EXCHANGE1    (01)    :        L00->R00, L01->R10, L10->R01, L11->R11 **/
/** EXCHANGE2    (10)    :        L00->R11, L01->R01, L10->R10, L11->R00 **/
/** EXHANGE3     (11)    :        L00->R10, L01->R01, L10->R00, L11->R11 **/

enum EStateTransRemapSchemes {
    ESTNOREMAP,
    ESTEXCHG1,
    ESTEXCHG2,
    ESTEXCHG3,
    ESTREMAPINVALID
    };

#define MAX_ENC_REMAP_COMBOS 9
int statetrans_remaps[MAX_ENC_REMAP_COMBOS][4][4] = {
    {
        {0, 1, 2, 3},   /*  NO REMAP    */
        {0, 2, 1, 3},   /*  EXCHANGE1   */
        {3, 1, 2, 0},   /*  EXCHANGE2   */
        {2, 1, 0, 3}    /*  EXCHANGE3   */
    },
    {
        {0, 1, 2, 3},   /*  NO REMAP    */
        {0, 3, 2, 1},   /*  EXCHANGE1   */
        {3, 1, 2, 0},   /*  EXCHANGE2   */
        {2, 1, 0, 3}    /*  EXCHANGE3   */
    },
    {
        {0, 1, 2, 3},   /*  NO REMAP    */
        {0, 2, 1, 3},   /*  EXCHANGE1   */
        {0, 3, 2, 1},   /*  EXCHANGE2   */
        {2, 1, 0, 3}    /*  EXCHANGE3   */
    },
    {
        {0, 1, 2, 3},   /*  NO REMAP    */
        {0, 2, 1, 3},   /*  EXCHANGE1   */
        {3, 1, 2, 0},   /*  EXCHANGE2   */
        {0, 3, 2, 1}    /*  EXCHANGE3   */
    },
    {
        {0, 3, 2, 1},   /*  NO REMAP    */
        {0, 2, 1, 3},   /*  EXCHANGE1   */
        {3, 1, 2, 0},   /*  EXCHANGE2   */
        {2, 1, 0, 3}    /*  EXCHANGE3   */
    },
    {
        {0, 1, 2, 3},   /*  NO REMAP    */
        {3, 2, 1, 0},   /*  FLIP ALL    */
        {2, 3, 0, 1},   /*  FLIP HI     */
        {2, 3, 1, 0}    /*  MIXED FLIP  */
    },
    {
        {0, 1, 2, 3},   /*  NO REMAP    */
        {0, 2, 1, 3},   /*  EXCHANGE1   */
        {3, 1, 2, 0},   /*  EXCHANGE2   */
        {2, 3, 0, 1}    /*  EXCHANGE3   */
    },
    {
        {0, 1, 2, 3},   /*  NO REMAP    */
        {0, 2, 1, 3},   /*  EXCHANGE1   */
        {3, 1, 2, 0},   /*  EXCHANGE2   */
        {3, 2, 1, 0}    /*  EXCHANGE3   */
    },
    {
        {3, 2, 1, 0},   /*  NO REMAP    */
        {0, 2, 1, 3},   /*  EXCHANGE1   */
        {2, 1, 0, 3},   /*  EXCHANGE2   */
        {2, 3, 0, 1}    /*  EXCHANGE3   */
    }

};
int st_trans_remap_energy_tbl[MAX_ENC_REMAP_COMBOS][4][4][4] = {
    /* Combo-1 */
    {
        /*  NO REMAP    */ {
            {00, 11, 22, 33},   /*  ZT  */
            {10, 01, 32, 23},   /*  ST  */
            {20, 30, 03, 13},   /*  HT  */
            {21, 31, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE1    */ {
            {00, 12, 21, 33},   /*  ZT  */
            {10, 02, 31, 23},   /*  ST  */
            {20, 30, 03, 13},   /*  HT  */
            {22, 32, 01, 11}    /*  TT  */
            },

        /*  EXCHANGE2    */ {
            {03, 11, 22, 30},   /*  ZT  */
            {13, 01, 32, 20},   /*  ST  */
            {23, 33, 00, 10},   /*  HT  */
            {21, 31, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE3    */ {
            {02, 11, 20, 33},   /*  ZT  */
            {12, 01, 30, 23},   /*  ST  */
            {22, 32, 03, 13},   /*  HT  */
            {21, 31, 00, 10}    /*  TT  */
            }
    },
    /* Combo-2 */
    {
        /*  NO REMAP    */ {
            {00, 11, 22, 33},   /*  ZT  */
            {10, 01, 32, 23},   /*  ST  */
            {20, 30, 03, 13},   /*  HT  */
            {21, 31, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE1    */ {
            {00, 13, 22, 31},   /*  ZT  */
            {10, 03, 32, 21},   /*  ST  */
            {20, 30, 01, 11},   /*  HT  */
            {23, 33, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE2    */ {
            {03, 11, 22, 30},   /*  ZT  */
            {13, 01, 32, 20},   /*  ST  */
            {23, 33, 00, 10},   /*  HT  */
            {21, 31, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE3    */ {
            {02, 11, 20, 33},   /*  ZT  */
            {12, 01, 30, 23},   /*  ST  */
            {22, 32, 03, 13},   /*  HT  */
            {21, 31, 00, 10}    /*  TT  */
            }
    },
    /* Combo-3 */
    {
        /*  NO REMAP    */ {
            {00, 11, 22, 33},   /*  ZT  */
            {10, 01, 32, 23},   /*  ST  */
            {20, 30, 03, 13},   /*  HT  */
            {21, 31, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE1    */ {
            {00, 12, 21, 33},   /*  ZT  */
            {10, 02, 31, 23},   /*  ST  */
            {20, 30, 03, 13},   /*  HT  */
            {22, 32, 01, 11}    /*  TT  */
            },

        /*  EXCHANGE2    */ {
            {00, 13, 22, 31},   /*  ZT  */
            {10, 03, 32, 21},   /*  ST  */
            {20, 30, 01, 11},   /*  HT  */
            {23, 33, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE3    */ {
            {02, 11, 20, 33},   /*  ZT  */
            {12, 01, 30, 23},   /*  ST  */
            {22, 32, 03, 13},   /*  HT  */
            {21, 31, 00, 10}    /*  TT  */
            },
    },
    /* Combo-4 */
    {
        /*  NO REMAP    */ {
            {00, 11, 22, 33},   /*  ZT  */
            {10, 01, 32, 23},   /*  ST  */
            {20, 30, 03, 13},   /*  HT  */
            {21, 31, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE1    */ {
            {00, 12, 21, 33},   /*  ZT  */
            {10, 02, 31, 23},   /*  ST  */
            {20, 30, 03, 13},   /*  HT  */
            {22, 32, 01, 11}    /*  TT  */
            },

        /*  EXCHANGE2    */ {
            {03, 11, 22, 30},   /*  ZT  */
            {13, 01, 32, 20},   /*  ST  */
            {23, 33, 00, 10},   /*  HT  */
            {21, 31, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE3    */ {
            {00, 13, 22, 31},   /*  ZT  */
            {10, 03, 32, 21},   /*  ST  */
            {20, 30, 01, 11},   /*  HT  */
            {23, 33, 02, 12}    /*  TT  */
            }
    },
    /* Combo-5 */
    {
        /*  NO REMAP    */ {
            {00, 13, 22, 31},   /*  ZT  */
            {10, 03, 32, 21},   /*  ST  */
            {20, 30, 01, 11},   /*  HT  */
            {23, 33, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE1    */ {
            {00, 12, 21, 33},   /*  ZT  */
            {10, 02, 31, 23},   /*  ST  */
            {20, 30, 03, 13},   /*  HT  */
            {22, 32, 01, 11}    /*  TT  */
            },

        /*  EXCHANGE2    */ {
            {03, 11, 22, 30},   /*  ZT  */
            {13, 01, 32, 20},   /*  ST  */
            {23, 33, 00, 10},   /*  HT  */
            {21, 31, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE3    */ {
            {02, 11, 20, 33},   /*  ZT  */
            {12, 01, 30, 23},   /*  ST  */
            {22, 32, 03, 13},   /*  HT  */
            {21, 31, 00, 10}    /*  TT  */
            }
    },
    /* Combo-6 */
    {
        /*  NO REMAP    */ {
            {00, 11, 22, 33},   /*  ZT  */
            {10, 01, 32, 23},   /*  ST  */
            {20, 30, 03, 13},   /*  HT  */
            {21, 31, 02, 12}    /*  TT  */
            },

        /*  FLIP ALL    */ {
            {03, 12, 21, 30},   /*  ZT  */
            {13, 02, 31, 20},   /*  ST  */
            {00, 10, 23, 33},   /*  HT  */
            {01, 11, 22, 32}    /*  TT  */
            },

        /*  FLIP HI     */ {
            {02, 13, 20, 31},   /*  ZT  */
            {12, 03, 30, 21},   /*  ST  */
            {22, 32, 01, 11},   /*  HT  */
            {23, 33, 00, 10}    /*  TT  */
            },

        /*  MIXED FLIP  */ {
            {20, 31, 12, 03},   /*  ZT  */
            {30, 21, 13, 02},   /*  ST  */
            {01, 11, 23, 33},   /*  HT  */
            {00, 10, 22, 32}    /*  TT  */
            }
    },
    /* Combo-7 */
    {
        /*  NO REMAP    */ {
            {00, 11, 22, 33},   /*  ZT  */
            {10, 01, 32, 23},   /*  ST  */
            {20, 30, 03, 13},   /*  HT  */
            {21, 31, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE1    */ {
            {00, 12, 21, 33},   /*  ZT  */
            {10, 02, 31, 23},   /*  ST  */
            {20, 30, 03, 13},   /*  HT  */
            {22, 32, 01, 11}    /*  TT  */
            },

        /*  EXCHANGE2    */ {
            {03, 11, 22, 30},   /*  ZT  */
            {13, 01, 32, 20},   /*  ST  */
            {23, 33, 00, 10},   /*  HT  */
            {21, 31, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE3    */ {
            {02, 13, 20, 31},   /*  ZT  */
            {12, 03, 30, 21},   /*  ST  */
            {22, 32, 01, 11},   /*  HT  */
            {23, 33, 00, 10}    /*  TT  */
            }
    },
    /* Combo-8 */
    {
        /*  NO REMAP    */ {
            {00, 11, 22, 33},   /*  ZT  */
            {10, 01, 32, 23},   /*  ST  */
            {20, 30, 03, 13},   /*  HT  */
            {21, 31, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE1    */ {
            {00, 12, 21, 33},   /*  ZT  */
            {10, 02, 31, 23},   /*  ST  */
            {20, 30, 03, 13},   /*  HT  */
            {22, 32, 01, 11}    /*  TT  */
            },

        /*  EXCHANGE2    */ {
            {03, 11, 22, 30},   /*  ZT  */
            {13, 01, 32, 20},   /*  ST  */
            {23, 33, 00, 10},   /*  HT  */
            {21, 31, 02, 12}    /*  TT  */
            },

        /*  EXCHANGE3    */ {
            {03, 12, 21, 30},   /*  ZT  */
            {13, 02, 31, 20},   /*  ST  */
            {00, 10, 23, 33},   /*  HT  */
            {01, 11, 22, 32}    /*  TT  */
            }
    },
    /* Combo-9 */
    {
        /*  NO REMAP    */ {
            {03, 12, 21, 30},   /*  ZT  */
            {13, 02, 31, 20},   /*  ST  */
            {00, 10, 23, 33},   /*  HT  */
            {01, 11, 22, 32}    /*  TT  */
            },

        /*  EXCHANGE1    */ {
            {00, 12, 21, 33},   /*  ZT  */
            {10, 02, 31, 23},   /*  ST  */
            {20, 30, 03, 13},   /*  HT  */
            {22, 32, 01, 11}    /*  TT  */
            },

        /*  EXCHANGE2    */ {
            {02, 11, 20, 33},   /*  ZT  */
            {12, 01, 30, 23},   /*  ST  */
            {22, 32, 03, 13},   /*  HT  */
            {21, 31, 00, 10}    /*  TT  */
            },

        /*  EXCHANGE3    */ {
            {02, 13, 20, 31},   /*  ZT  */
            {12, 03, 30, 21},   /*  ST  */
            {22, 32, 01, 11},   /*  HT  */
            {23, 33, 00, 10}    /*  TT  */
            }
    }
};

#if 0
#if REMAP_COMBO == 1
int statetrans_remaps[4][4] = { {0, 1, 2, 3},   /*  NO REMAP    */
                                {0, 2, 1, 3},   /*  EXCHANGE1   */
                                {3, 1, 2, 0},   /*  EXCHANGE2   */
                                {2, 1, 0, 3}    /*  EXCHANGE3   */    };


int st_trans_remap_energy_tbl[4][4][4] = {
    /*  NO REMAP    */ {
        {00, 11, 22, 33},   /*  ZT  */
        {10, 01, 32, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE1    */ {
        {00, 12, 21, 33},   /*  ZT  */
        {10, 02, 31, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {22, 32, 01, 11}    /*  TT  */
        },

    /*  EXCHANGE2    */ {
        {03, 11, 22, 30},   /*  ZT  */
        {13, 01, 32, 20},   /*  ST  */
        {23, 33, 00, 10},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE3    */ {
        {02, 11, 20, 33},   /*  ZT  */
        {12, 01, 30, 23},   /*  ST  */
        {22, 32, 03, 13},   /*  HT  */
        {21, 31, 00, 10}    /*  TT  */
        }
};
#elif REMAP_COMBO == 2
int statetrans_remaps[4][4] = { {0, 1, 2, 3},   /*  NO REMAP    */
                                {0, 3, 2, 1},   /*  EXCHANGE1   */
                                {3, 1, 2, 0},   /*  EXCHANGE2   */
                                {2, 1, 0, 3}    /*  EXCHANGE3   */    };


int st_trans_remap_energy_tbl[4][4][4] = {
    /*  NO REMAP    */ {
        {00, 11, 22, 33},   /*  ZT  */
        {10, 01, 32, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE1    */ {
        {00, 13, 22, 31},   /*  ZT  */
        {10, 03, 32, 21},   /*  ST  */
        {20, 30, 01, 11},   /*  HT  */
        {23, 33, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE2    */ {
        {03, 11, 22, 30},   /*  ZT  */
        {13, 01, 32, 20},   /*  ST  */
        {23, 33, 00, 10},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE3    */ {
        {02, 11, 20, 33},   /*  ZT  */
        {12, 01, 30, 23},   /*  ST  */
        {22, 32, 03, 13},   /*  HT  */
        {21, 31, 00, 10}    /*  TT  */
        }
};
#elif REMAP_COMBO == 3
int statetrans_remaps[4][4] = { {0, 1, 2, 3},   /*  NO REMAP    */
                                {0, 2, 1, 3},   /*  EXCHANGE1   */
                                {0, 3, 2, 1},   /*  EXCHANGE2   */
                                {2, 1, 0, 3}    /*  EXCHANGE3   */    };


int st_trans_remap_energy_tbl[4][4][4] = {
    /*  NO REMAP    */ {
        {00, 11, 22, 33},   /*  ZT  */
        {10, 01, 32, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE1    */ {
        {00, 12, 21, 33},   /*  ZT  */
        {10, 02, 31, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {22, 32, 01, 11}    /*  TT  */
        },

    /*  EXCHANGE2    */ {
        {00, 13, 22, 31},   /*  ZT  */
        {10, 03, 32, 21},   /*  ST  */
        {20, 30, 01, 11},   /*  HT  */
        {23, 33, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE3    */ {
        {02, 11, 20, 33},   /*  ZT  */
        {12, 01, 30, 23},   /*  ST  */
        {22, 32, 03, 13},   /*  HT  */
        {21, 31, 00, 10}    /*  TT  */
        },
};
#elif REMAP_COMBO == 4
int statetrans_remaps[4][4] = { {0, 1, 2, 3},   /*  NO REMAP    */
                                {0, 2, 1, 3},   /*  EXCHANGE1   */
                                {3, 1, 2, 0},   /*  EXCHANGE2   */
                                {0, 3, 2, 1}    /*  EXCHANGE3   */    };


int st_trans_remap_energy_tbl[4][4][4] = {
    /*  NO REMAP    */ {
        {00, 11, 22, 33},   /*  ZT  */
        {10, 01, 32, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE1    */ {
        {00, 12, 21, 33},   /*  ZT  */
        {10, 02, 31, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {22, 32, 01, 11}    /*  TT  */
        },

    /*  EXCHANGE2    */ {
        {03, 11, 22, 30},   /*  ZT  */
        {13, 01, 32, 20},   /*  ST  */
        {23, 33, 00, 10},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE3    */ {
        {00, 13, 22, 31},   /*  ZT  */
        {10, 03, 32, 21},   /*  ST  */
        {20, 30, 01, 11},   /*  HT  */
        {23, 33, 02, 12}    /*  TT  */
        }
};
#elif REMAP_COMBO == 5 //No NOREMAP - State Transforming all
int statetrans_remaps[4][4] = { {0, 3, 2, 1},   /*  NO REMAP    */
                                {0, 2, 1, 3},   /*  EXCHANGE1   */
                                {3, 1, 2, 0},   /*  EXCHANGE2   */
                                {2, 1, 0, 3}    /*  EXCHANGE3   */    };


int st_trans_remap_energy_tbl[4][4][4] = {
    /*  NO REMAP    */ {
        {00, 13, 22, 31},   /*  ZT  */
        {10, 03, 32, 21},   /*  ST  */
        {20, 30, 01, 11},   /*  HT  */
        {23, 33, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE1    */ {
        {00, 12, 21, 33},   /*  ZT  */
        {10, 02, 31, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {22, 32, 01, 11}    /*  TT  */
        },

    /*  EXCHANGE2    */ {
        {03, 11, 22, 30},   /*  ZT  */
        {13, 01, 32, 20},   /*  ST  */
        {23, 33, 00, 10},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE3    */ {
        {02, 11, 20, 33},   /*  ZT  */
        {12, 01, 30, 23},   /*  ST  */
        {22, 32, 03, 13},   /*  HT  */
        {21, 31, 00, 10}    /*  TT  */
        }
};
#elif REMAP_COMBO == 6 //same as state-preserving
int statetrans_remaps[4][4] = { {0, 1, 2, 3},   /*  NO REMAP    */
                                {3, 2, 1, 0},   /*  FLIP ALL    */
                                {2, 3, 0, 1},   /*  FLIP HI     */
                                {2, 3, 1, 0}    /*  MIXED FLIP  */    };


int st_trans_remap_energy_tbl[4][4][4] = {
    /*  NO REMAP    */ {
        {00, 11, 22, 33},   /*  ZT  */
        {10, 01, 32, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  FLIP ALL    */ {
        {03, 12, 21, 30},   /*  ZT  */
        {13, 02, 31, 20},   /*  ST  */
        {00, 10, 23, 33},   /*  HT  */
        {01, 11, 22, 32}    /*  TT  */
        },

    /*  FLIP HI     */ {
        {02, 13, 20, 31},   /*  ZT  */
        {12, 03, 30, 21},   /*  ST  */
        {22, 32, 01, 11},   /*  HT  */
        {23, 33, 00, 10}    /*  TT  */
        },

    /*  MIXED FLIP  */ {
        {20, 31, 12, 03},   /*  ZT  */
        {30, 21, 13, 02},   /*  ST  */
        {01, 11, 23, 33},   /*  HT  */
        {00, 10, 22, 32}    /*  TT  */
        }
};
#elif REMAP_COMBO == 7                          //remove {0, 3, 2, 1} and {2, 1, 0, 3} to add FLIP-HI
int statetrans_remaps[4][4] = { {0, 1, 2, 3},   /*  NO REMAP    */
                                {0, 2, 1, 3},   /*  EXCHANGE1   */
                                {3, 1, 2, 0},   /*  EXCHANGE2   */
                                {2, 3, 0, 1}    /*  EXCHANGE3   */    };


int st_trans_remap_energy_tbl[4][4][4] = {
    /*  NO REMAP    */ {
        {00, 11, 22, 33},   /*  ZT  */
        {10, 01, 32, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE1    */ {
        {00, 12, 21, 33},   /*  ZT  */
        {10, 02, 31, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {22, 32, 01, 11}    /*  TT  */
        },

    /*  EXCHANGE2    */ {
        {03, 11, 22, 30},   /*  ZT  */
        {13, 01, 32, 20},   /*  ST  */
        {23, 33, 00, 10},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE3    */ {
        {02, 13, 20, 31},   /*  ZT  */
        {12, 03, 30, 21},   /*  ST  */
        {22, 32, 01, 11},   /*  HT  */
        {23, 33, 00, 10}    /*  TT  */
        }
};
#elif REMAP_COMBO == 8                          //remove {0, 3, 2, 1} and {2, 1, 0, 3} to add FLIP-HI
int statetrans_remaps[4][4] = { {0, 1, 2, 3},   /*  NO REMAP    */
                                {0, 2, 1, 3},   /*  EXCHANGE1   */
                                {3, 1, 2, 0},   /*  EXCHANGE2   */
                                {3, 2, 1, 0}    /*  EXCHANGE3   */    };


int st_trans_remap_energy_tbl[4][4][4] = {
    /*  NO REMAP    */ {
        {00, 11, 22, 33},   /*  ZT  */
        {10, 01, 32, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE1    */ {
        {00, 12, 21, 33},   /*  ZT  */
        {10, 02, 31, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {22, 32, 01, 11}    /*  TT  */
        },

    /*  EXCHANGE2    */ {
        {03, 11, 22, 30},   /*  ZT  */
        {13, 01, 32, 20},   /*  ST  */
        {23, 33, 00, 10},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  EXCHANGE3    */ {
        {03, 12, 21, 30},   /*  ZT  */
        {13, 02, 31, 20},   /*  ST  */
        {00, 10, 23, 33},   /*  HT  */
        {01, 11, 22, 32}    /*  TT  */
        }
};
#else                                           // No no-remap, 2 state preserving and FLIP-HI and Flip - All
int statetrans_remaps[4][4] = { {3, 2, 1, 0},   /*  NO REMAP    */
                                {0, 2, 1, 3},   /*  EXCHANGE1   */
                                {2, 1, 0, 3},   /*  EXCHANGE2   */
                                {2, 3, 0, 1}    /*  EXCHANGE3   */    };


int st_trans_remap_energy_tbl[4][4][4] = {
    /*  NO REMAP    */ {
        {03, 12, 21, 30},   /*  ZT  */
        {13, 02, 31, 20},   /*  ST  */
        {00, 10, 23, 33},   /*  HT  */
        {01, 11, 22, 32}    /*  TT  */
        },

    /*  EXCHANGE1    */ {
        {00, 12, 21, 33},   /*  ZT  */
        {10, 02, 31, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {22, 32, 01, 11}    /*  TT  */
        },

    /*  EXCHANGE2    */ {
        {02, 11, 20, 33},   /*  ZT  */
        {12, 01, 30, 23},   /*  ST  */
        {22, 32, 03, 13},   /*  HT  */
        {21, 31, 00, 10}    /*  TT  */
        },

    /*  EXCHANGE3    */ {
        {02, 13, 20, 31},   /*  ZT  */
        {12, 03, 30, 21},   /*  ST  */
        {22, 32, 01, 11},   /*  HT  */
        {23, 33, 00, 10}    /*  TT  */
        }
};
#endif // REMAP_COMBO
#endif // 0
/* Currently we calculate actual energy with this floating point calculation *
 * We need to modify this function for calculating cost intelligently. At the*
 * least we can use integers in approximate ratio of energies for various   *
 * energy transitions                                                        */
double MLC::energy_cost(bool statepreserving, uint8_t aRemapScheme, std::unordered_map<int, int>& aCountMap) {
    int i=0, j=0, s=0;
    double energy = 0;
    int (*tbl_ptr)[4];
    tbl_ptr = (statepreserving == true) ? remap_energy_tbl[aRemapScheme] : st_trans_remap_energy_tbl[enc_remap_op][aRemapScheme];

    for(i=0; i < 4; i++) {
        s = 0;
        for (j=0; j < 4; j++) {
            s += aCountMap[tbl_ptr[i][j]];
        }
        energy += energy_val[i]*s;
    }
    return energy;
}

void MLC::increment_state_transitions(bool statepreserving, uint8_t aRemapScheme, std::unordered_map<int, int>& aCountMap, std::vector<int>& res) {
    int i=0, j=0;
    int (*tbl_ptr)[4];
    tbl_ptr = (statepreserving == true) ? remap_energy_tbl[aRemapScheme] : st_trans_remap_energy_tbl[enc_remap_op][aRemapScheme];

    for(i=0; i < 4; i++) {
        for (j=0; j < 4; j++) {
            res[i] += aCountMap[tbl_ptr[i][j]];
        }
    }
}

void MLC::update_energy_profile(uint8_t aRemapScheme, std::unordered_map<int, int>& aCountMap, Stats::Vector& bucket) {
    int i=0, j=0;
    int (*tbl_ptr)[4] = remap_energy_tbl[aRemapScheme];

    for(i=0; i < 4; i++) {
        for (j=0; j < 4; j++) {
            bucket[i] += aCountMap[tbl_ptr[i][j]];
        }
    }
    /* Now update the total count of this transition */
    bucket[4]++;
}

bool MLC::isencodingU(const Byte* chunk, int chunkSize, uint8_t remapScheme, int thres) {
    int hibit = 0;
    for(int i=0; i < chunkSize; i++) {
        for(int j=0; j < 8; j+=2) {
            int before = (chunk[i] >> j) & 3;
            int after = statetrans_remaps[enc_remap_op][remapScheme][before];
            hibit = hibit + ((after >> 1) & 1);
        }
    }
    return (hibit > thres or hibit < (chunkSize*4 - thres));
}


std::vector<int> MLC::lineCompare_2bit_enc_based_mapping(const Byte* ablock, const Byte* bblock, int size, int shiftSize, int flipSize, int flipBits, int thres){
    int mask = 0;
    int fs = flipSize; // the size the chunk , if flipsize iis 0 means no flip
    if( flipSize == 0 ) fs = size+1; // no flip
    else
        mask = 2*(size/fs) - 2;
    std::unordered_map<int, int> normal_cnt;
    std::vector<int> ret(5, 512);
    ret[4] = 0;
    Byte from, to;
    std::vector<int> res(5,0);// ZT, ST, HT, TT, remapping bits
    int encA = generate_exact_encoding(ablock, size, encodingSize, thres, flipBits);
    int encB = generate_encoding(bblock, size, encodingSize, thres);
    int ifFlip = (flipBits >>mask) &3; // ifFlip records the flip option information, because in the block I stored the real content, we need remmaped it back to cell content.
    int encbitA = (encA >> (mask/2)) & 1;
    int encbitB = (encB >> (mask/2)) & 1;
    for(int j = 0; j < size; j++){ // for each byte
        from = ablock[j]; // from block
        to = bblock[j]; // to is the new block

        for(int k = 0; k<8; k += 2)  { // for each 2-bit inside a byte
            int label_fr = (((from >> k) & 3));
            int label_to =  ((to >> k) & 3); // 3 for 0b11

            int label = (encbitA == 1) ? ((stateful_remaps[ifFlip][label_fr])*10 + label_to) : ((statetrans_remaps[enc_remap_op][ifFlip][label_fr])*10 + label_to); // so we use label to indicate the 2-bit old cell content and 2-bit new real content
            normal_cnt[label] += 1;
        }

        if(fs <= size && (j+1)% fs == 0 ){ // when the byte is multiple of chunk size , need to decide if we flip this chunk
        //Flip decision is made by the count. Rakesh you need change the decision logic here
            double min_energy = 256*energy_val[ETT]; //Some high value to start with
            uint8_t finalScheme = 0;
            uint8_t start_scheme = (encbitB == 1) ? (uint8_t)ENOREMAP : (uint8_t)ESTNOREMAP;
            uint8_t end_scheme = (encbitB == 1) ? (uint8_t)EREMAPINVALID : (uint8_t)ESTREMAPINVALID;
            for(uint8_t scheme = start_scheme; scheme < end_scheme; scheme++) {
                double temp = energy_cost(encbitB, scheme, normal_cnt);
                if(temp < min_energy) {
                    finalScheme = scheme;
                    min_energy = temp;
                }
            }
            /* Check if the final scheme ensures change of state to "U" for this "D" chunk */
            if((encbitB == 0) && (finalScheme != 0) && (enc_correction == true) &&
               !(isencodingU(bblock+j+1-fs, fs, finalScheme, thres)))
                finalScheme = 0;

            increment_state_transitions(encbitB, finalScheme, normal_cnt, res);
            res[4] = ((res[4] << 2) | finalScheme);
            mask = mask -2;
            ifFlip = (flipBits >> mask) & 3; // update the ifPlip for next chunk
            encbitA = (encA >> (mask/2)) & 1; //update enc for next chunk
            encbitB = (encB >> (mask/2)) & 1; //update enc for next chunk
            normal_cnt.clear();
        }
    }
    if( fs > size){// no filp at all
        for(auto it : normal_cnt){
            if(it.first == 0 or it.first == 33 or it.first == 22 or it.first == 11 )//zt
                res[0] += it.second;
            else if(it.first == 1 or it.first == 10 or it.first == 32 or it.first == 23)//st
                res[1] += it.second;
            else if(it.first == 3 or it.first == 13 or it.first == 20 or it.first == 30) //ht
                res[2] += it.second;
            else
                res[3] += it.second;
            }
            res[4] = 0;
    }
    ret = res;
    assert(ret[3] + ret[2] + ret[0] + ret[1] == 256);
    return ret;
}


std::vector<int> MLC::lineCompare_2bit_stateful_mapping( const Byte* ablock, const Byte* bblock, int size, int shiftSize, int flipSize, int flipBits){
    int mask = 0;
    int fs = flipSize; // the size the chunk , if flipsize iis 0 means no flip
    if( flipSize == 0 ) fs = size+1; // no flip
    else
        mask = 2*(size/fs) - 2;
    std::unordered_map<int, int> normal_cnt;
    std::vector<int> ret(5, 512);
    ret[4] = 0;
    Byte from, to;
    std::vector<int> res(5,0);// ZT, ST, HT, TT & I use the the res[4] to return the new flip bits string
    int ifFlip = (flipBits >>mask) &3; // ifFlip records the flip option information, because in the block I stored the real content, we need remmaped it back to cell content.
    for(int j = 0; j < size; j++){ // for each byte
        from = ablock[j]; // from block
        to = bblock[j]; // to is the new block


        for(int k = 0; k<8; k += 2)  { // for each 2-bit inside a byte
            int label_fr = (((from >> k) & 3));
            int label_to =  ((to >> k) & 3); // 3 for 0b11

            int label = (stateful_remaps[ifFlip][label_fr])*10 + label_to; // so we use label to indicate the 2-bit old cell content and 2-bit new real content
            normal_cnt[label] += 1;
        }

        if(fs <= size && (j+1)% fs == 0 ){ // when the byte is multiple of chunk size , need to decide if we flip this chunk
        //Flip decision is made by the count. Rakesh you need change the decision logic here
            double min_energy = 256*energy_val[ETT]; //Some high value to start with
            uint8_t finalScheme = 0;
            for(uint8_t scheme = ENOREMAP; scheme < EREMAPINVALID; scheme++) {
                double temp = energy_cost(true, scheme, normal_cnt);
                if(temp < min_energy) {
                    finalScheme = scheme;
                    min_energy = temp;
                }
            }
            res[4] = ((res[4] << 2) | finalScheme);
            increment_state_transitions(true, finalScheme, normal_cnt, res);

            /* Get the encoding for chunk at same position of incoming and outgoing block to give a two-tuple from 0-3 */
            int trans_idx = (curr_blk_enc_trans >> 30) & 3;
            switch(trans_idx) {
            case 0:
                //DD
                update_energy_profile(finalScheme, normal_cnt, dd_energy_profile);
                break;
            case 1:
                //DU
                update_energy_profile(finalScheme, normal_cnt, du_energy_profile);
                break;
            case 2:
                //UD
                update_energy_profile(finalScheme, normal_cnt, ud_energy_profile);
                break;
            case 3:
                //UU
                update_energy_profile(finalScheme, normal_cnt, uu_energy_profile);
                break;
            default:
                assert(1);
            }
            curr_blk_enc_trans <<= 2;

            mask = mask -2;
            ifFlip = (flipBits >> mask) & 3; // update the ifPlip for next chunk
            normal_cnt.clear();
        }
    }
    if( fs > size){// no filp at all
        for(auto it : normal_cnt){
            if(it.first == 0 or it.first == 33 or it.first == 22 or it.first == 11 )//zt
                res[0] += it.second;
            else if(it.first == 1 or it.first == 10 or it.first == 32 or it.first == 23)//st
                res[1] += it.second;
            else if(it.first == 3 or it.first == 13 or it.first == 20 or it.first == 30) //ht
                res[2] += it.second;
            else
                res[3] += it.second;
            }
            res[4] = 0;
    }
    ret = res;
    assert(ret[3] + ret[2] + ret[0] + ret[1] == 256);
    return ret;
}

/* Block level mapping schemes */
/* Hi-Flip - ON */
/** FLIP ALL        (00)    :        L00->R11, L01->R10, L10->R01, L11->R00 **/
/** FLIP HI         (01)    :        L00->R10, L01->R11, L10->R00, L11->R01 **/
/** MIXED FLIP-3    (10)    :        L00->R10, L01->R11, L10->R01, L11->R00 **/
/** MIXED FLIP-4    (11)    :        L00->R11, L01->R10, L10->R00, L11->R01 **/

/* Hi-Flip - OFF */
/** NO REMAP        (00)    :        L00->R00, L01->R01, L10->R10, L11->R11 **/
/** FLIP LO         (01)    :        L00->R01, L01->R00, L10->R11, L11->R10 **/
/** MIXED FLIP-1    (10)    :        L00->R00, L01->R01, L10->R11, L11->R10 **/
/** MIXED FLIP-2    (11)    :        L00->R01, L01->R00, L10->R10, L11->R11 **/

enum EHiFlipRemapSchemes {
    EHIFLIPALL,
    EHIFLIPHI,
    EHIFLIPMIXED3,
    EHIFLIPMIXED4,
    EHIFLIPREMAPINVALID
};

int hiflip_stateful_remaps[8][4] = {    {3, 2, 1, 0},   /*  FLIP ALL        */
                                        {2, 3, 0, 1},   /*  FLIP HI         */
                                        {2, 3, 1, 0},   /*  MIXED FLIP - 3  */
                                        {3, 2, 0, 1}    /*  MIXED FLIP - 4  */    };

int hiflip_remap_energy_tbl[4][4][4] = {
    /*  FLIP ALL        */ {
        {03, 12, 21, 30},   /*  ZT  */
        {13, 02, 31, 20},   /*  ST  */
        {23, 33, 00, 10},   /*  HT  */
        {22, 32, 01, 11}    /*  TT  */
        },

    /*  FLIP HI         */ {
        {02, 13, 20, 31},   /*  ZT  */
        {12, 03, 30, 21},   /*  ST  */
        {22, 32, 01, 11},   /*  HT  */
        {23, 33, 00, 10}    /*  TT  */
        },

    /*  FLIP MIXED-3     */ {
        {02, 13, 21, 30},   /*  ZT  */
        {12, 03, 31, 20},   /*  ST  */
        {22, 32, 00, 10},   /*  HT  */
        {23, 33, 01, 11}    /*  TT  */
        },

    /*  FLIP MIXED-4     */ {
        {03, 12, 20, 31},   /*  ZT  */
        {13, 02, 30, 21},   /*  ST  */
        {23, 33, 01, 11},   /*  HT  */
        {22, 32, 00, 10}    /*  TT  */
        }
};

enum ELoFlipRemapSchemes {
    ELONOREMAP,
    ELOFLIPLO,
    ELOFLIPMIXED1,
    ELOFLIPMIXED2,
    ELOFLIPREMAPINVALID
};

int loflip_stateful_remaps[4][4] = {    {0, 1, 2, 3},   /*  NO REMAP        */
                                        {1, 0, 3, 2},   /*  FLIP LO         */
                                        {0, 1, 3, 2},   /*  MIXED FLIP - 1  */
                                        {1, 0, 2, 3},   /*  MIXED FLIP - 2  */  };

int loflip_remap_energy_tbl[4][4][4] = {
    /*  NO REMAP        */ {
        {00, 11, 22, 33},   /*  ZT  */
        {10, 01, 32, 23},   /*  ST  */
        {20, 30, 03, 13},   /*  HT  */
        {21, 31, 02, 12}    /*  TT  */
        },

    /*  FLIP LO         */ {
        {01, 10, 23, 32},   /*  ZT  */
        {11, 00, 33, 22},   /*  ST  */
        {21, 31, 02, 12},   /*  HT  */
        {20, 30, 03, 13}    /*  TT  */
        },

    /*  FLIP MIXED-1     */ {
        {00, 11, 23, 32},   /*  ZT  */
        {10, 01, 33, 22},   /*  ST  */
        {20, 30, 02, 12},   /*  HT  */
        {21, 31, 03, 13}    /*  TT  */
        },

    /*  FLIP MIXED-2     */ {
        {01, 10, 22, 33},   /*  ZT  */
        {11, 00, 32, 23},   /*  ST  */
        {21, 31, 03, 13},   /*  HT  */
        {20, 30, 02, 12}    /*  TT  */
        }
};

/* Currently we calculate actual energy with this floating point calculation *
 * We need to modify this function for calculating cost intelligently. At the*
 * least we can use integers in approximate ratio of energies for various   *
 * energy transitions                                                        */
double blk_energy_cost(bool hiflip, uint8_t aRemapScheme, std::unordered_map<int, int>& aCountMap) {
    int i=0, j=0, s=0;
    int (*tbl_ptr)[4];
    tbl_ptr = (hiflip == true) ? hiflip_remap_energy_tbl[aRemapScheme] : loflip_remap_energy_tbl[aRemapScheme];
    double energy = 0;

    for(i=0; i < 4; i++) {
        s = 0;
        for (j=0; j < 4; j++) {
            s += aCountMap[tbl_ptr[i][j]];
        }
        energy += energy_val[i]*s;
    }
    return energy;
}

void increment_blk_state_transitions(bool hiflip, uint8_t aRemapScheme, std::unordered_map<int, int>& aCountMap, std::vector<int>& res) {
    int i=0, j=0;
    int (*tbl_ptr)[4];
    tbl_ptr = (hiflip == true) ? hiflip_remap_energy_tbl[aRemapScheme] : loflip_remap_energy_tbl[aRemapScheme];

    for(i=0; i < 4; i++) {
        for (j=0; j < 4; j++) {
            res[i] += aCountMap[tbl_ptr[i][j]];
        }
    }
}

std::vector<int> MLC::lineCompare_blk_mapping( const Byte* ablock, const Byte* bblock, int size, int shiftSize, int flipSize, int flipBits){
    std::unordered_map<int, int> normal_cnt;
    std::vector<int> ret(5, 512);
    std::vector<int> res(5,0);// ZT, ST, HT, TT & I use the the res[4] to return the new flip bits string
    ret[4] = 0;
    if(flipSize) {
        double min_energy = 0;
        uint8_t finalScheme = 0;
        int scheme = 0;
        int mask = (size/flipSize) - 1;
        //Byte *hiFlipToChunks, *hiFlipFromChunks, *loFlipToChunks, *loFlipFromChunks;
        std::vector<Byte> hiFlipToChunks, hiFlipFromChunks, loFlipToChunks, loFlipFromChunks;
        std::vector<Byte> currToChunk(flipSize, 0), currFromChunk(flipSize, 0);
        int loflip = flipBits & 3; //lowest 2 bits for lo-flip remapping chunks
        flipBits >>= 2; //add args to function for blk based flip bit sizes
        int hiflip = flipBits & 3;
        flipBits >>= 2; //add args to function for blk based flip bit sizes
        int ifFlip = (flipBits >> mask) &1; // ifFlip records the flip option information, because in the block I stored the real content, we need remmaped it back to cell content.
        Byte from, to;
        int q = 0;
        int hicount = 0;

        for(int j = 0; j < size; j++){ // for each byte
            from = ablock[j]; // from block
            to = bblock[j]; // to is the new block
            currToChunk[q] = to; //blindly copy current to block data

            for(int k = 0; k<8; k += 2)  { // for each 2-bit inside a byte
                int label_fr = (((from >> k) & 3));
                int label_to =  ((to >> k) & 3); // 3 for 0b11
                if(ifFlip) {
                    label_fr = hiflip_stateful_remaps[hiflip][label_fr];
                }
                else {
                    label_fr = loflip_stateful_remaps[loflip][label_fr];
                }
                currFromChunk[q] = (currFromChunk[q] << 2) | label_fr;
                if(((label_to ^ label_fr) & 2 ) == 0)
                    hicount++; //record that for this 2-bit tuple both to and from have same high order bit
            }
            q++;

            if((j+1)% flipSize == 0 ){
                if(hicount < 2*flipSize) {
                    /* Set flip bit for this chunk */
                    for (uint8_t m = 0; m < flipSize; m++) {
                        hiFlipFromChunks.push_back(currFromChunk[m]);
                        hiFlipToChunks.push_back(currToChunk[m]);
                    }
                    res[4] = (res[4] << 1 | 1);
                }
                else {
                    /* No flip bit for this chunk */
                    for (uint8_t m = 0; m < flipSize; m++) {
                        loFlipFromChunks.push_back(currFromChunk[m]);
                        loFlipToChunks.push_back(currToChunk[m]);
                    }
                    res[4] = (res[4] << 1);
                }
                currToChunk.clear();
                currFromChunk.clear();
                q = 0;
                hicount = 0;
                mask = mask -1;
                ifFlip = (flipBits >> mask) & 1; // update the ifPlip for next chunk
            }
        }
        for (uint8_t m = 0; m < hiFlipFromChunks.size(); m++) {
            for(int u = 0; u<8; u += 2)  { // for each 2-bit inside a byte
                int label = (((hiFlipFromChunks[m] >> u) & 3)*10) + ((hiFlipToChunks[m] >> u) & 3);
                normal_cnt[label] += 1;
            }
        }

        min_energy = 256*energy_val[ETT]; //Some high value to start with
        finalScheme = 0;
        for(scheme = EHIFLIPALL; scheme < EHIFLIPREMAPINVALID; scheme++) {
            double temp = blk_energy_cost(true, scheme, normal_cnt);
            if(temp < min_energy) {
                finalScheme = scheme;
                min_energy = temp;
            }
        }
        res[4] = ((res[4] << 2) | finalScheme);
        increment_blk_state_transitions(true, finalScheme, normal_cnt, res);

        normal_cnt.clear();
        for (uint8_t m = 0; m < loFlipFromChunks.size(); m++) {
            for(int u = 0; u<8; u += 2)  { // for each 2-bit inside a byte
                int label = (((loFlipFromChunks[m] >> u) & 3)*10) + ((loFlipToChunks[m] >> u) & 3);
                normal_cnt[label] += 1;
            }
        }

        min_energy = 256*energy_val[ETT]; //Some high value to start with
        finalScheme = 0;
        for(scheme = ELONOREMAP; scheme < ELOFLIPREMAPINVALID; scheme++) {
            double temp = blk_energy_cost(false, scheme, normal_cnt);
            if(temp < min_energy) {
                finalScheme = scheme;
                min_energy = temp;
            }
        }
        res[4] = ((res[4] << 2) | finalScheme);
        increment_blk_state_transitions(false, finalScheme, normal_cnt, res);
    }
    else {// no filp at all
            normal_cnt.clear();
            for (uint8_t m = 0; m < size; m++) {
                for(int u = 0; u<8; u += 2)  { // for each 2-bit inside a byte
                    int label = (((ablock[m] >> u) & 3)*10) + ((bblock[m] >> u) & 3);
                    normal_cnt[label] += 1;
                }
            }
            increment_blk_state_transitions(false, ELONOREMAP, normal_cnt, res); //force no remap option
            res[4] = 0;
    }
    ret = res;
    assert(ret[3] + ret[2] + ret[0] + ret[1] == 256);
    return ret;
}

std::vector<int> MLC::lineCompare_2bit( const Byte* ablock, const Byte* bblock, int size, int shiftSize, int flipSize, int flipBits){
	int mask = 1;
	int fs = flipSize;
	if( flipSize == 0 ) fs = size+1; // no flip
	else
		mask = 1<<( (size/fs) -1 );
	std::unordered_map<int, int> normal_cnt;
	std::unordered_map<int, int> rev_cnt;
	std::vector<int> ret(5, 512);
	ret[4] = 0;
	//int minimal_bits = 512;
	//for (int i =0; i<size; i += shiftSize){
		Byte from, to;
		std::vector<int> res(5,0);// ZT, ST, HT, TT
		//int total_bits = 0;
		//int flipbits = 0;
		bool ifFlip = flipBits & mask;
		int cnt01 = 0;
		int cnt10 = 0;
		for(int j = 0; j < size; j++){
			from = ablock[j]; // from block
			if(ifFlip) from = ~from;
			to = bblock[j]; // to block


			for(int k = 0; k<8; k += 2)  {
				int label = (((from >> k) & 3)*10) + ((to >> k) & 3); // 3 for 0b11
				int rev_label = (((from >> k) & 3)*10) + 3 - ((to >> k) & 3);
				if(label  == 30 or label == 31 or label == 20 or label == 21) cnt10++; // significant bits change from 1 to 0
				else if(label == 3 or label == 13 or label == 2 or label == 12 ) cnt01 ++; // significant bits change from 0 to 1
				//std::cout<<"label "<< label << " rev " << rev_label<<std::endl;
				normal_cnt[label] += 1;
				rev_cnt[rev_label] += 1;// 3 is the mask 0x11. count every 2 bits in a byte

			}

			if(fs <= size && (j+1)% fs == 0 ){ // to decide if flip
				if(cnt01 + cnt10 <= fs*2){ // no flip this time

					res[4] = res[4]<<1 ;
					//bool ifSwap = false;
					/*if( normal_cnt[1] + normal_cnt[11] + normal_cnt[22] + normal_cnt[32] < normal_cnt[2] + normal_cnt[12] +normal_cnt[21] +normal_cnt[31]){
						//ifSwap = true;
						std::swap(normal_cnt[1], normal_cnt[2]);
						std::swap(normal_cnt[11], normal_cnt[12]);
						std::swap(normal_cnt[21], normal_cnt[22]);
						std::swap(normal_cnt[31], normal_cnt[32]);
					}*/
					for(auto it : normal_cnt){
						if(it.first == 0 or it.first == 33 or it.first == 22 or it.first == 11 )//zt
							res[0] += it.second;
						else if(it.first == 1 or it.first == 10 or it.first == 32 or it.first == 23)//st
							res[1] += it.second;
						else if(it.first == 3 or it.first == 13 or it.first == 20 or it.first == 30) //ht
							res[2] += it.second;
						else
							res[3] += it.second; //tt
					}
				}else{
					res[4] = (res[4]<<1) + 1;
					/*if( rev_cnt[1] + rev_cnt[11] + rev_cnt[22] + rev_cnt[32] < rev_cnt[2] + rev_cnt[12] + rev_cnt[21] + rev_cnt[31]){
						//ifSwap = true;
						std::swap(rev_cnt[1], rev_cnt[2]);
						std::swap(rev_cnt[11], rev_cnt[12]);
						std::swap(rev_cnt[21], rev_cnt[22]);
						std::swap(rev_cnt[31], rev_cnt[32]);
					}*/
					for(auto it : rev_cnt){
						if(it.first == 0 or it.first == 33 or it.first == 22 or it.first == 11 )//zt
							res[0] += it.second;
						else if(it.first == 1 or it.first == 10 or it.first == 32 or it.first == 23)//st
							res[1] += it.second;
						else if(it.first == 3 or it.first == 13 or it.first == 20 or it.first == 30) //ht
							res[2] += it.second;
						else
							res[3] += it.second;
					}
				}
				mask = mask >> 1;
				ifFlip = flipBits & mask;
				normal_cnt.clear();
				rev_cnt.clear();
				cnt01 = 0;
				cnt10 = 0;
			}
		}
		if( fs > size){// no filp at all
				for(auto it : normal_cnt){
						if(it.first == 0 or it.first == 33 or it.first == 22 or it.first == 11 )//zt
							res[0] += it.second;
						else if(it.first == 1 or it.first == 10 or it.first == 32 or it.first == 23)//st
							res[1] += it.second;
						else if(it.first == 3 or it.first == 13 or it.first == 20 or it.first == 30) //ht
							res[2] += it.second;
						else
							res[3] += it.second;
						}
						res[4] = 0;
		}

		//if(res[3] + res[2] < ret[3] + ret[2])
		ret = res;
	//}
	assert(ret[3] + ret[2] + ret[0] + ret[1] == 256);
	return ret;
}
/*
std::vector<int>
MLC::lineCompare_2bit( const Byte* ablock, const Byte* bblock, int size, int shiftSize, int flipSize, int encodingBits){
	if( flipSize == 0 ) flipSize = size+1; // no flip
	std::unordered_map<int, int> normal_cnt;
	std::unordered_map<int, int> rev_cnt;
	std::vector<int> ret(5, 512);
	//int minimal_bits = 512;
	for (int i =0; i<size; i += shiftSize){
		Byte from, to;
		std::vector<int> res(5,0);// ZT, ST, HT, TT
		//int total_bits = 0;
		//int flipbits = 0;
		bool ifFlip = encodingBits & 1;
		int cnt01 = 0;
		int cnt10 = 0;
		for(int j = 0; j < size; j++){
			from = ablock[j]; // from block
			if(ifFlip) from = ~ from;
			to = bblock[(j+i)%size]; // to block


			for(int k = 0; k<8; k += 2)  {
				int label = (((from >> k) & 3)*10) + ((to >> k) & 3); // 3 for 0b11
				int rev_label = (((from >> k) & 3)*10) + ~((to >> k) & 3);
				if(label  == 30 or label == 31 or label == 20 or label == 21) cnt10++; // significant bits change from 1 to 0
				else if(label == 3 or label == 13 or label == 2 or label == 12 ) cnt01 ++;
				normal_cnt[label]++;
				rev_cnt[rev_label]++;// 3 is the mask 0x11. count every 2 bits in a byte

			}
			if((j+1)% flipSize == 0 ){ // to decide if flip
				if(cnt01 + cnt10 <= flipSize*2){
					res[4] = res[4]<<1 ;
					for(auto it : normal_cnt){
						if(it.first == 0 or it.first == 33 or it.first == 22 or it.first == 11 )//zt
							res[0] += it.second;
						else if(it.first == 1 or it.first == 10 or it.first == 32 or it.first == 23)//st
							res[1] += it.second;
						else if(it.first == 3 or it.first == 13 or it.first == 20 or it.first == 30) //ht
							res[2] += it.second;
						else
							res[3] += it.second;
					}
				}else{
					res[4] = (res[4]<<1) + 1;
					for(auto it : rev_cnt){
						if(it.first == 0 or it.first == 33 or it.first == 22 or it.first == 11 )//zt
							res[0] += it.second;
						else if(it.first == 1 or it.first == 10 or it.first == 32 or it.first == 23)//st
							res[1] += it.second;
						else if(it.first == 3 or it.first == 13 or it.first == 20 or it.first == 30) //ht
							res[2] += it.second;
						else
							res[3] += it.second;
					}
				}
				ifFlip = encodingBits & (1 << ((j+1)/flipSize));
				normal_cnt.clear();
				rev_cnt.clear();
				cnt01 = 0;
				cnt10 = 0;
			}
		}
		if(flipSize > size){
				for(auto it : normal_cnt){
						if(it.first == 0 or it.first == 33 or it.first == 22 or it.first == 11 )//zt
							res[0] += it.second;
						else if(it.first == 1 or it.first == 10 or it.first == 32 or it.first == 23)//st
							res[1] += it.second;
						else if(it.first == 3 or it.first == 13 or it.first == 20 or it.first == 30) //ht
							res[2] += it.second;
						else
							res[3] += it.second;
				}
		}
		if(res[3] + res[2] < ret[3] + ret[2])
				ret = res;
	}
	assert(ret[3] + ret[2] + ret[0] + ret[1] == 256);
	return ret;
}
*/

int
MLC::encodingCompare(const Byte* ablock, const Byte* bblock, int size, int shiftSize, int flipSize, int thres, int encodingSize){
	if(flipSize == 0 ) flipSize = size+1;
	int minimal_diff = size/8;
	for (int i =0; i<size; i += shiftSize) {
		//Byte temp;
		int total_diff = 0;
		int abits = 0, bbits = 0;
		for(int j = 0; j<size; j++){
				for(int k = 0; k<8; k++)  {
					if (ablock[j] & (1<<k) ) abits++; // count every bit in a byte
					if (bblock[j] & (1<<k) ) bbits++;
					}
				if( (j+1)%encodingSize == 0 ) {
					if(abits > thres or abits < (encodingSize*8 - thres) ) abits = 1;
					else abits = 0;
					if(bbits > thres or bbits < (encodingSize*8 - thres) ) bbits = 1;
					else bbits = 0;
					if(abits != bbits) total_diff++;
					abits = 0;
					bbits = 0;
				}
		}
		//total_bits += flipbits;  // if no flip..
		if(total_diff < minimal_diff) minimal_diff = total_diff;
	}
	//assert(minimal_diff <= size/encodingSize);
	return minimal_diff;
}

CacheBlk*
MLC::findVictim(Addr addr, PacketPtr pkt)
{
    //int set = extractSet(addr);
    // grab a replacement candidate
    //CacheBlk *blk = BaseSetAssoc::findVictim(addr);
    //bool unfilled = true ;

    //LRU framework
	int set = extractSet(addr);
    // grab a replacement candidate
    BlkType *blk = nullptr;
    //BlkType *ret = nullptr;
    //int cur_min = INT_MAX;
    //std::vector<int> lru_trans;
   // std::vector<int> opt_trans;
    std::vector<int> fb(5,0);
    int numSeg =  (64 / encodingSize) + 1;
   // bool lru = true;
    //int rind = assoc - 1;
   /* for (int i = assoc - 1; i >= 0; i--) { // replace the second to LRU
        BlkType *b = sets[set].blks[i];
        if (b->way < allocAssoc) {
            blk = b;
			lru_trans = lineCompare_2bit(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[blk->way]);
			if(lru){
				for(int i =0; i < 4; i++)
						lruTrans[i] += lru_trans[i];
				lru = false;
				if(i != assoc -1){
					opt_trans = lru_trans;
					//cur_min = lru_trans[2]+lru_trans[3];
					rind = i;
					ret = blk;
					break;
				}

			}
			if(lru_trans[2]+lru_trans[3] < cur_min){
				opt_trans = lru_trans;
				cur_min = lru_trans[2]+lru_trans[3];
				rind = i;
				ret = blk;
			}
        }
    }
    for(int i = 0; i < 4 ; i++)
		optimalTrans[i] += opt_trans[i];

	sets[set].flipBits[ret->way] = 	opt_trans[4];
    rind = rind;
    assert(!ret || ret->way < allocAssoc);

    if (ret && ret->isValid()) {
        DPRINTF(CacheRepl, "set %x: selecting blk %x for replacement at %d\n", set, regenerateBlkAddr(ret->tag, set), rind);
    }
    return ret;*/

    std::vector<int> locVal(assoc, 0);
    //LRU ENDS
    int retValue = -1;
    int idx =  assoc - 1;
    for (; idx >= 0; idx--) { // replace the second to LRU
        BlkType *b = sets[set].blks[idx];
        if ( !b->isValid() && b->way < allocAssoc) {
            blk = b;
            break;
        }
        blk = b;
    }
    //idx ++;
    if(idx < 0 ) idx = 0;

	//MLC replacement process (no invalid block to replace)
	if ( blk && blk->isValid()) {
		//int retValue = -1;
		if(range == 8){
			int plru[16] = {0};
			plru[0] = sets[set].m_tree[0];
			plru[1] = sets[set].m_tree[1];
			plru[2] = sets[set].m_tree[16];
			plru[3] = sets[set].m_tree[2];
			plru[4] = sets[set].m_tree[9];
			plru[5] = sets[set].m_tree[17];
			plru[6] = sets[set].m_tree[24];
			for(int i = 7; i<15; i++){
				int temp = 0;
				int p = i;
				int level = 0;
				while( p != 0){
					if(p%2==0 && plru[p/2-1] ==1) temp = temp+ (1<<level);
					else if( p%2 == 1 && plru[p/2] == 0) temp = temp + (1<<level);
					level ++;
					p = (p-1)/2;
				}
				if(temp == 7 ) // temp higher, more lru
					plru[i] = 1; // LRU one, the plru value is smallest as 1.
				else if(temp == 6)
					plru[i] = 2;
				else if(temp > 3)
					plru[i] = 3;
				else
					plru[i] = 4;
				}

				 if ( sets[set].m_tree[3] == 0)
				 {
					if ( sets[set].m_tree[4] == 0) retValue= 0;
					else           retValue= 1;  // b2==1
				 }
				 else
				 {                            // b1==1
					if ( sets[set].m_tree[5] == 0) retValue = 2;
					else           retValue = 3;  // b3==1
				 }
				 locVal[retValue]=plru[7];                         // b0==1
				 if ( sets[set].m_tree[6] == 0)
				 {
					if ( sets[set].m_tree[7] == 0) retValue = 4;
					else           retValue = 5;  // b5==1
				 }
				 else
				 {                            // b4==1
					if ( sets[set].m_tree[8] == 0) retValue = 6;
					else           retValue = 7;  // b6==1
				}
				locVal[retValue] = (plru[8]);

				 if ( sets[set].m_tree[10] == 0)
				 {
					if ( sets[set].m_tree[11] == 0) retValue= 8;
					else           retValue= 9;  // b2==1
				 }
				 else
				 {                            // b1==1
					if ( sets[set].m_tree[12] == 0) retValue = 10;
					else           retValue = 11;  // b3==1
				 }
					locVal[retValue] = (plru[9]);   // b0==1
				 if ( sets[set].m_tree[13] == 0)
				 {
					if ( sets[set].m_tree[14] == 0) retValue = 12;
					else           retValue = 13;  // b5==1
				 }
				 else
				 {                            // b4==1
					if ( sets[set].m_tree[15] == 0) retValue = 14;
					else           retValue = 15;  // b6==1
				 }
				locVal[retValue] = (plru[10]);

				 if ( sets[set].m_tree[18] == 0)
				 {
					if ( sets[set].m_tree[19] == 0) retValue= 16;
					else            retValue= 17;  // b2==1
				 }
				 else
				 {                            // b1==1
					if ( sets[set].m_tree[20] == 0) retValue = 18;
					else           retValue = 19;  // b3==1
				 }
				locVal[retValue] = (plru[11]);                               // b0==1
				 if ( sets[set].m_tree[21] == 0)
				 {
					if ( sets[set].m_tree[22] == 0) retValue = 20;
					else           retValue = 21;  // b5==1
				 }
				 else
				 {                            // b4==1
					if ( sets[set].m_tree[23] == 0) retValue = 22;
					else           retValue = 23;  // b6==1
				 }
				locVal[retValue] = (plru[12]);

				 if ( sets[set].m_tree[25] == 0)
				 {
					if ( sets[set].m_tree[26] == 0) retValue= 24;
					else           retValue= 25;  // b2==1
				 }
				 else
				 {                            // b1==1
					if ( sets[set].m_tree[27] == 0) retValue = 26;
					else           retValue = 27;  // b3==1
				 }
				locVal[retValue] = (plru[13]);
										   // b0==1
				 if ( sets[set].m_tree[28] == 0)
				 {
					if ( sets[set].m_tree[29] == 0) retValue = 28;
					else           retValue = 29;  // b5==1
				 }
				 else
				 {                            // b4==1
					if ( sets[set].m_tree[30] == 0) retValue = 30;
					else           retValue = 31;  // b6==1
				 }
				locVal[retValue] = (plru[14]);
				}
				if(range == 4){
					int plru[8] = {0};
					plru[0] = sets[set].m_tree[0];
					plru[1] = sets[set].m_tree[1];
					plru[2] = sets[set].m_tree[16];

					for(int i = 3; i<7; i++){
						int temp = 0;
						int p = i;
						int level = 0;
						while( p != 0){
							if(p%2==0 && plru[p/2-1] ==1) temp = temp+ (1<<level);
							else if( p%2 == 1 && plru[p/2] == 0) temp = temp + (1<<level);
							level ++;
							p = (p-1)/2;
						}
						if(temp == 3 ) // they are the rank of regions
							plru[i] = 1;
						else if(temp == 2)
							plru[i] = 2;
						else if(temp ==1)
							plru[i] = 2;
						else
							plru[i] = 2;
						}


					  if ( sets[set].m_tree[2] == 0)
					  {
						 if ( sets[set].m_tree[3] == 0)
						 {
							if ( sets[set].m_tree[4] == 0) retValue= 0;
							else           retValue= 1;  // b2==1
						 }
						 else
						 {                            // b1==1
							if ( sets[set].m_tree[5] == 0) retValue = 2;
							else           retValue = 3;  // b3==1
						 }
					  }
					  else
					  {
						 if ( sets[set].m_tree[6] == 0)
						 {                               // b0==1
							if ( sets[set].m_tree[7] == 0) retValue = 4;
							else           retValue = 5;  // b5==1
						 }
						 else
						 {                            // b4==1
							if ( sets[set].m_tree[8] == 0) retValue = 6;
							else           retValue = 7;  // b6==1
						 }
					  }
						locVal[retValue] = (plru[3]);


					  if ( sets[set].m_tree[9] == 0)
					  {
						 if ( sets[set].m_tree[10] == 0)
						 {
							if ( sets[set].m_tree[11] == 0) retValue= 8;
							else           retValue= 9;  // b2==1
						 }
						 else
						 {                            // b1==1
							if ( sets[set].m_tree[12] == 0) retValue = 10;
							else           retValue = 11;  // b3==1
						 }
					  }
					  else
					  {                               // b0==1
						 if ( sets[set].m_tree[13] == 0)
						 {
							if ( sets[set].m_tree[14] == 0) retValue = 12;
							else           retValue = 13;  // b5==1
						 }
						 else
						 {                            // b4==1
							if ( sets[set].m_tree[15] == 0) retValue = 14;
							else           retValue = 15;  // b6==1
						 }
					  }
						locVal[retValue] = (plru[4]);

					  if ( sets[set].m_tree[17] == 0)
					  {
						 if ( sets[set].m_tree[18] == 0)
						 {
							if ( sets[set].m_tree[19] == 0) retValue= 16;
							else            retValue= 17;  // b2==1
						 }
						 else
						 {                            // b1==1
							if ( sets[set].m_tree[20] == 0) retValue = 18;
							else           retValue = 19;  // b3==1
						 }
					  }
					  else
					  {                               // b0==1
						 if ( sets[set].m_tree[21] == 0)
						 {
							if ( sets[set].m_tree[22] == 0) retValue = 20;
							else           retValue = 21;  // b5==1
						 }
						 else
						 {                            // b4==1
							if ( sets[set].m_tree[23] == 0) retValue = 22;
							else           retValue = 23;  // b6==1
						 }
					  }

						locVal[retValue] = (plru[5]);
					  if ( sets[set].m_tree[24] == 0)
					  {
						 if ( sets[set].m_tree[25] == 0)
						 {
							if ( sets[set].m_tree[26] == 0) retValue= 24;
							else           retValue= 25;  // b2==1
						 }
						 else
						 {                            // b1==1
							if ( sets[set].m_tree[27] == 0) retValue = 26;
							else           retValue = 27;  // b3==1
						 }
					  }
					  else
					  {                               // b0==1
						 if ( sets[set].m_tree[28] == 0)
						 {
							if ( sets[set].m_tree[29] == 0) retValue = 28;
							else           retValue = 29;  // b5==1
						 }
						 else
						 {                            // b4==1
							if ( sets[set].m_tree[30] == 0) retValue = 30;
							else           retValue = 31;  // b6==1
						 }
					  }
					locVal[retValue] = (plru[6]);
				}
				 int min_index = 0;
				 int enc_d = 0;
				 int cur_hd = 0;
				 int cur_recency = 0;
				 double lowestDiffBits = 9999999;
				 //double low_en = 0;
				std::priority_queue<double, std::vector<double>, std::greater<double> > pq;
				std::vector<int> curfb(5,0);
			if(options == 4 ){
				for(UInt32 i = 0 ; i < assoc; i++)
				{
					int recency = locVal[i];
						if(recency == 0) recency = 10;
						//if( range == 0 && recency == 1) recency = 0;
						//set->read_line(i, 0, read_buff, 64, false);
						//std::memcpy(blk->data, pkt->getConstPtr<uint8_t>(), blkSize);
						//enc_d = encodingCompare_2bit(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, thres, encodingSize);


							if(loc_weight >= 1024 ) //old
								curfb = lineCompare(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]); // old line_compare
							else if(loc_weight >= 512 ) // 2bit remmaping
								//curfb = lineCompare_2bit_mapping(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
								curfb = lineCompare_blk_mapping(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
							else // flip MSB 1 bit
								curfb = lineCompare_2bit(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
								//pq.push(1.084*fb[1]+ 1.084*fb[3] + 2.653*fb[2] + 2.653*fb[3]);


							double cur = 1.92*curfb[1]+ 1.92*curfb[3] + 3.192*curfb[2] + 3.192*curfb[3]; // new change here ???
							//if(enc_d < 7 )
							//	cur = 0 + recency * (loc_weight % 512);
							//int t = (8 * encodingSize) * hd  + recency * loc_weight;
							//std::cout<< i <<" diff "<<t<<" bits "<<(uint8_t)read_buff[0]<<std::endl;
							if(cur < lowestDiffBits || ( cur == lowestDiffBits && recency < cur_recency)) {
								fb = curfb;
								lowestDiffBits = cur;
								min_index = i;
								//cur_hd = enc_d;
								cur_recency = recency;
							}
						}
						totalZT[273] += fb[0];
						totalST[273] += fb[1];
						totalHT[273] += fb[2];
						totalTT[273] += fb[3];
						totalReps[273] += 1;

						idx = min_index;
						blk = sets[set].blks[idx];
						sets[set].flipBits[idx] = fb[4];
						while (blk->way >= allocAssoc) {
							idx = (idx + 1) % assoc;
							blk = sets[set].blks[idx];
						}

						assert(idx < assoc);
						assert(idx >= 0);
						assert(blk->way < allocAssoc);


			}
			else if(options == 2  or options == 3 ){
				for(UInt32 i = 0 ; i < assoc; i++)
				{
					int recency = locVal[i];
					if(recency != 0 or options == 3 ){
						if(recency == 0) recency = 10;
						//if( range == 0 && recency == 1) recency = 0;
						//set->read_line(i, 0, read_buff, 64, false);
						//std::memcpy(blk->data, pkt->getConstPtr<uint8_t>(), blkSize);
						enc_d = encodingCompare_2bit(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, thres, encodingSize);


							if(loc_weight >= 1024 ) //old
								curfb = lineCompare(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]); // old line_compare
							else if(loc_weight >= 512 ) // 2bit remmaping
								//curfb = lineCompare_2bit_mapping(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
								curfb = lineCompare_blk_mapping(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
							else // flip MSB 1 bit
								curfb = lineCompare_2bit(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
								//pq.push(1.084*fb[1]+ 1.084*fb[3] + 2.653*fb[2] + 2.653*fb[3]);
							totalZT[enc_d] += curfb[0];
							totalST[enc_d] += curfb[1];
							totalHT[enc_d] += curfb[2];
							totalTT[enc_d] += curfb[3];
							//avgZT[enc_d] = curfb[0];
							//avgST[enc_d] = curfb[1];
							//avgHT[enc_d] = curfb[2];
							//avgTT[enc_d] = curfb[3];
							totalReps[enc_d] += 1;
							double cur = 10*(enc_d/numSeg) + diverse_weight*(enc_d%numSeg) + recency * (loc_weight % 512);
							if(options == 4)
								cur = 1.92*curfb[1]+ 1.92*curfb[3] + 3.192*curfb[2] + 3.192*curfb[3];
							//if(enc_d < 7 )
							//	cur = 0 + recency * (loc_weight % 512);
							//int t = (8 * encodingSize) * hd  + recency * loc_weight;
							//std::cout<< i <<" diff "<<t<<" bits "<<(uint8_t)read_buff[0]<<std::endl;
							if(cur < lowestDiffBits || ( cur == lowestDiffBits && recency < cur_recency)) {
								fb = curfb;
								lowestDiffBits = cur;
								min_index = i;
								cur_hd = enc_d;
								cur_recency = recency;
							}
						}
					}
						idx = min_index;
						blk = sets[set].blks[idx];
						sets[set].flipBits[idx] = fb[4];
						while (blk->way >= allocAssoc) {
							idx = (idx + 1) % assoc;
							blk = sets[set].blks[idx];
						}

						assert(idx < assoc);
						assert(idx >= 0);
						assert(blk->way < allocAssoc);


			}
			else if(options == 0 or options == 1 or options == 5 or options == 6 or options == 7 or options == 8 or options == 9 or options == 10){ // options 1 for rank options 0 for normal
                int lru_index = -1;
				for(UInt32 i = 0 ; i < assoc; i++)
				{
					int recency = locVal[i];
					if(recency != 0 ){
					    if(recency == 1)
                            lru_index = i;
						//if( range == 0 && recency == 1) recency = 0;
						//set->read_line(i, 0, read_buff, 64, false);
						//std::memcpy(blk->data, pkt->getConstPtr<uint8_t>(), blkSize);
						if((options != 9) && (options != 10))
                            enc_d = encodingCompare_2bit(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, thres, encodingSize);
                        else
                            enc_d = encodingCompare_exact(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, sets[set].flipBits[idx], encodingSize, thres);

						if(options == 1 or options == 7 or options == 8 or options == 10){ // curfb counts the tt zt and ht
							/*if(loc_weight >= 1024 )
								curfb = lineCompare(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]); // old line_compare
							else if(loc_weight >= 512 ) // 2bit*/
								//curfb = lineCompare_2bit_mapping(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
							if (options == 1)
								curfb = lineCompare_2bit_mapping(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
							else if (options == 7)
								curfb = lineCompare_2bit_stateful_mapping(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
							else if(options == 8)
								curfb = lineCompare_blk_mapping(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
                            else //options == 10
                                curfb = lineCompare_2bit_enc_based_mapping(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx], thres);
							/*else // flip MSB 1 bit
								curfb = lineCompare_2bit(sets[set].blks[i]->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);*/
							pq.push(1.92*curfb[1]+ 1.92*curfb[3] + 3.192*curfb[2] + 3.192*curfb[3]);
						}
						int numD = getNumD(sets[set].blks[i]->data, 64, encodingSize, thres);
						double cur = 10*(enc_d/numSeg) + diverse_weight*(enc_d%numSeg) + recency * (loc_weight % 512) + tie_weight*numD;

					//	if(enc_d < 7 )
					//		cur = 0 + recency * (loc_weight % 512);
						//int t = (8 * encodingSize) * hd  + recency * loc_weight;
						//std::cout<< i <<" diff "<<t<<" bits "<<(uint8_t)read_buff[0]<<std::endl;
						if(cur < lowestDiffBits || ( cur == lowestDiffBits && recency < cur_recency)) {
						//std::cout<< i <<" diff "<<t<<std::endl;
							//low_en = 1.084*fb[1]+ 1.084*fb[3] + 2.653*fb[2] + 2.653*fb[3];
							fb = curfb;
							lowestDiffBits = cur;
							min_index = i;
							cur_hd = enc_d;
							cur_recency = recency;
						}
					}
				}
				int rank = 1;
				if( options == 1 or options == 7 or options == 8 or options == 10)
					while( !pq.empty() && (1.92*fb[1]+ 1.92*fb[3] + 3.192*fb[2] + 3.192*fb[3]) > pq.top()){
						pq.pop();
						rank++;
					}
				totalRanks[rank] += 1;
				if(diverse_weight == 10.0) {
                    int numUU = numSeg - ((cur_hd/numSeg) + (cur_hd%numSeg) + 1);
                    if(numUU > UUthres)
                        idx = min_index;
                    else
                        idx = lru_index;
				}
				else
                    idx = min_index;
				blk = sets[set].blks[idx];

				while (blk->way >= allocAssoc) {
					idx = (idx + 1) % assoc;
					blk = sets[set].blks[idx];
				}

				assert(idx < assoc);
				assert(idx >= 0);
				assert(blk->way < allocAssoc);

		//assert(!blk || blk->way < allocAssoc);
				if(options == 0 or options == 5 or options == 6 or options == 9) {
					/*if(loc_weight >= 1024 )
						fb = lineCompare(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]); // old line_compare
					else if(loc_weight >= 512 ) // 2bit*/
						//fb = lineCompare_2bit_mapping(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
					if(options == 0)
						fb = lineCompare_2bit_mapping(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
					else if(options == 5)
						fb = lineCompare_2bit_stateful_mapping(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
					else if(options == 6)
						fb = lineCompare_blk_mapping(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
                    else //options == 9
                        fb = lineCompare_2bit_enc_based_mapping(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx], thres);
					/*else // flip MSB 1 bit
						fb = lineCompare_2bit(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);*/
				}
			//std::cout<<cur_hd<<std::endl;
		//totalFlipbits[cur_hd] += fb;
				totalZT[cur_hd] += fb[0];
				totalST[cur_hd] += fb[1];
				totalHT[cur_hd] += fb[2];
				totalTT[cur_hd] += fb[3];
				totalReps[enc_d] += 1;
				sets[set].flipBits[idx] = fb[4];
				//totalZT[numSeg*numSeg + 1] += fb[0];
				//totalST[numSeg*numSeg + 1] += fb[1];
				//totalHT[numSeg*numSeg + 1] += fb[2];
				//totalZT[numSeg*numSeg + 1] += fb[3];

				//avgZT[cur_hd] = fb[0];
				//avgST[cur_hd] = fb[1];
				//avgHT[cur_hd] = fb[2];
				//avgTT[cur_hd] = fb[3];

				//avgZT[numSeg*numSeg + 1] = fb[0];
				//avgST[numSeg*numSeg + 1] = fb[1];
				//avgHT[numSeg*numSeg + 1] = fb[2];
				//avgTT[numSeg*numSeg + 1] = fb[3];
					//avgFlipbits[cur_hd] = fb;
				DPRINTF(CacheRepl, "set %x: selecting blk %x for plru replacement with hd = %d %d %d %d \n", set, regenerateBlkAddr(blk->tag, set), cur_hd, fb[0], fb[1], fb[2], fb[3]);
			}
		}else{
            #if 0 //we can choose various mappings per option - don't need loc_weight for selection right now
				if(loc_weight >= 1024 )
					fb = lineCompare(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]); // old line_compare
				else if(loc_weight >= 512 ) // 2bit
					//fb = lineCompare_2bit_mapping(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
					fb = lineCompare_blk_mapping(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
				else // flip MSB 1 bit
					fb = lineCompare_2bit(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
            #endif

                if((options == 0) or (options == 1))
                    fb = lineCompare_2bit_mapping(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
                else if((options == 5) or (options == 7))
                    fb = lineCompare_2bit_stateful_mapping(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
                else if((options == 6) or (options == 8))
                    fb = lineCompare_blk_mapping(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);
                else if ((options == 9) or (options == 0))
                    fb = lineCompare_2bit_enc_based_mapping(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx], thres);
				else // flip MSB 1 bit - Note sure if we use this any more but just as fallback case
					fb = lineCompare_2bit(blk->data, pkt->getConstPtr<uint8_t>(), 64, shiftSize, flipSize, sets[set].flipBits[idx]);


                sets[set].flipBits[idx] = fb[4];
                totalZT[271] += fb[0];
                totalST[271] += fb[1];
                totalHT[271] += fb[2];
                totalTT[271] += fb[3];
                totalInvalidFill += 1;
                //avgZT[numSeg*numSeg - 1] = fb[0];
                //avgST[numSeg*numSeg - 1] = fb[1];
                //avgHT[numSeg*numSeg - 1] = fb[2];
                //avgTT[numSeg*numSeg - 1] = fb[3];
                //avgZT[numSeg*numSeg + 1] = fb[0];
                //avgST[numSeg*numSeg + 1] = fb[1];
                //avgHT[numSeg*numSeg + 1] = fb[2];
                //avgTT[numSeg*numSeg + 1] = fb[3];
                DPRINTF(CacheRepl, "set %x: selecting blk %x for plru replacement a invalid one with hd = %d %d %d %d \n", set, regenerateBlkAddr(blk->tag, set), 0, fb[0], fb[1], fb[2], fb[3]);
	}
		//DPRINTF(CacheRepl, "set %x: selecting blk %x for plru replacement with \n", set, regenerateBlkAddr(blk->tag, set));

		return blk;
}



CacheBlk*
MLC::findVictimPLRU(Addr addr, PacketPtr pkt)
{
    int set = extractSet(addr);
    // grab a replacement candidate
    BlkType *blk = nullptr;

    for (int i = assoc - 1; i >= 0; i--) {
        BlkType *b = sets[set].blks[i];
        if (b->way < allocAssoc) {
            blk = b;
            break;
        }
    }
    assert(!blk || blk->way < allocAssoc);
    int retValue = -1;

    if (blk && blk->isValid()) {//replacement process


	   if( assoc == 1 ) retValue = 0;
	   if (assoc == 4)
	   {
		  if (sets[set].m_tree[0] == 0)
		  {
			 if (sets[set].m_tree[1] == 0) retValue = 0;
			 else           retValue = 1;   // b1==1
		  }
		  else
		  {
			 if (sets[set].m_tree[2] == 0) retValue = 2;
			 else           retValue = 3;   // b2==1
		  }
	   }
	   else if (assoc == 8)
	   {
		  if (sets[set].m_tree[0] == 0)
		  {
			 if (sets[set].m_tree[1] == 0)
			 {
				if (sets[set].m_tree[2] == 0) retValue= 0;
				else           retValue= 1;  // b2==1
			 }
			 else
			 {                            // b1==1
				if (sets[set].m_tree[3] == 0) retValue = 2;
				else           retValue = 3;  // b3==1
			 }
		  }
		  else
		  {                               // b0==1
			 if (sets[set].m_tree[4] == 0)
			 {
				if (sets[set].m_tree[5] == 0) retValue = 4;
				else           retValue = 5;  // b5==1
			 }
			 else
			 {                            // b4==1
				if (sets[set].m_tree[6] == 0) retValue = 6;
				else           retValue = 7;  // b6==1
			 }
		  }
	   }
	   else if (assoc == 16)
	   {
		  if (sets[set].m_tree[0] == 0)
			  {
				  if (sets[set].m_tree[1] == 0)
				  {
					 if (sets[set].m_tree[2] == 0)
					 {
						if (sets[set].m_tree[3] == 0) retValue= 0;
						else           retValue= 1;  // b2==1
					 }
					 else
					 {                            // b1==1
						if (sets[set].m_tree[4] == 0) retValue = 2;
						else           retValue = 3;  // b3==1
					 }
				  }
				  else
				  {                               // b0==1
					 if (sets[set].m_tree[5] == 0)
					 {
						if (sets[set].m_tree[6] == 0) retValue = 4;
						else           retValue = 5;  // b5==1
					 }
					 else
					 {                            // b4==1
						if (sets[set].m_tree[7] == 0) retValue = 6;
						else           retValue = 7;  // b6==1
					 }
				  }
				}
				else{
				  if (sets[set].m_tree[8] == 0)
				  {
					 if (sets[set].m_tree[9] == 0)
					 {
						if (sets[set].m_tree[10] == 0) retValue= 8;
						else           retValue= 9;  // b2==1
					 }
					 else
					 {                            // b1==1
						if (sets[set].m_tree[11] == 0) retValue = 10;
						else           retValue = 11;  // b3==1
					 }
				  }
				  else
				  {                               // b0==1
					 if (sets[set].m_tree[12] == 0)
					 {
						if (sets[set].m_tree[13] == 0) retValue = 12;
						else           retValue = 13;  // b5==1
					 }
					 else
					 {                            // b4==1
						if (sets[set].m_tree[14] == 0) retValue = 14;
						else           retValue = 15;  // b6==1
					 }
				  }
				}
			}
		else if (assoc == 32)
		{
			  if( sets[set].m_tree[0] == 0 ){

				  if( sets[set].m_tree[1] == 0 )
				  {
					  if (sets[set].m_tree[2] == 0)
					  {
						 if (sets[set].m_tree[3] == 0)
						 {
							if (sets[set].m_tree[4] == 0) retValue= 0;
							else           retValue= 1;  // b2==1
						 }
						 else
						 {                            // b1==1
							if (sets[set].m_tree[5] == 0) retValue = 2;
							else           retValue = 3;  // b3==1
						 }
					  }
					  else
					  {                               // b0==1
						 if (sets[set].m_tree[6] == 0)
						 {
							if (sets[set].m_tree[7] == 0) retValue = 4;
							else           retValue = 5;  // b5==1
						 }
						 else
						 {                            // b4==1
							if (sets[set].m_tree[8] == 0) retValue = 6;
							else           retValue = 7;  // b6==1
						 }
					  }
					}
					else{
					  if (sets[set].m_tree[9] == 0)
					  {
						 if (sets[set].m_tree[10] == 0)
						 {
							if (sets[set].m_tree[11] == 0) retValue= 8;
							else           retValue= 9;  // b2==1
						 }
						 else
						 {                            // b1==1
							if (sets[set].m_tree[12] == 0) retValue = 10;
							else           retValue = 11;  // b3==1
						 }
					  }
					  else
					  {                               // b0==1
						 if (sets[set].m_tree[13] == 0)
						 {
							if (sets[set].m_tree[14] == 0) retValue = 12;
							else           retValue = 13;  // b5==1
						 }
						 else
						 {                            // b4==1
							if (sets[set].m_tree[15] == 0) retValue = 14;
							else           retValue = 15;  // b6==1
						 }
					  }
					}
				}
				else{
					if( sets[set].m_tree[16] == 0 )
					{
					  if (sets[set].m_tree[17] == 0)
					  {
						 if (sets[set].m_tree[18] == 0)
						 {
							if (sets[set].m_tree[19] == 0) retValue= 16;
							else            retValue= 17;  // b2==1
						 }
						 else
						 {                            // b1==1
							if (sets[set].m_tree[20] == 0) retValue = 18;
							else           retValue = 19;  // b3==1
						 }
					  }
					  else
					  {                               // b0==1
						 if (sets[set].m_tree[21] == 0)
						 {
							if (sets[set].m_tree[22] == 0) retValue = 20;
							else           retValue = 21;  // b5==1
						 }
						 else
						 {                            // b4==1
							if (sets[set].m_tree[23] == 0) retValue = 22;
							else           retValue = 23;  // b6==1
						 }
					  }
					}
					else{
					  if (sets[set].m_tree[24] == 0)
					  {
						 if (sets[set].m_tree[25] == 0)
						 {
							if (sets[set].m_tree[26] == 0) retValue= 24;
							else           retValue= 25;  // b2==1
						 }
						 else
						 {                            // b1==1
							if (sets[set].m_tree[27] == 0) retValue = 26;
							else           retValue = 27;  // b3==1
						 }
					  }
					  else
					  {                               // b0==1
						 if (sets[set].m_tree[28] == 0)
						 {
							if (sets[set].m_tree[29] == 0) retValue = 28;
							else           retValue = 29;  // b5==1
						 }
						 else
						 {                            // b4==1
							if (sets[set].m_tree[30] == 0) retValue = 30;
							else           retValue = 31;  // b6==1
						 }
					  }
					}
				}
			}

	 blk = sets[set].blks[retValue];
     DPRINTF(CacheRepl, "set %x: selecting blk %x for plru replacement\n",
                set, regenerateBlkAddr(blk->tag, set));
    }

    return blk;
}

void
MLC::insertBlock(PacketPtr pkt, BlkType *blk)
{
    BaseSetAssoc::insertBlock(pkt, blk);

    int set = extractSet(pkt->getAddr());
    sets[set].moveToFront(blk);
}

void
MLC::invalidate(CacheBlk *blk)
{
    BaseSetAssoc::invalidate(blk);

    // should be evicted before valid blocks
   // int set = blk->set;
    //sets[set].moveToTail(blk);
}

MLC*
MLCParams::create()
{
    return new MLC(this);
}
