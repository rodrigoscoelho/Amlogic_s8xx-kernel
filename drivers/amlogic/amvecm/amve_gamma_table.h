/*
 * Video Enhancement
 *
 * Author: dezhi kong <lin.xu@amlogic.com>
 *
 * Copyright (C) 2014 Amlogic Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __AM_VE_GAMMATABLE_H
#define __AM_VE_GAMMATABLE_H

#include <linux/amlogic/amports/amstream.h>

struct tcon_gamma_table_s video_curve_2d2_inv =
{
	{
	   0,	82,  113,  136,  155,  171,  186,  199,  212,  223,  234,  245,  255,  264,  273,  282,
	 290,  298,  306,  314,  321,  328,  335,  342,  349,  356,  362,  368,  374,  380,  386,  392,
	 398,  403,  409,  414,  420,  425,  430,  435,  440,  445,  450,  455,  460,  464,  469,  474,
	 478,  483,  487,  492,  496,  500,  505,  509,  513,  517,  521,  525,  529,  533,  537,  541,
	 545,  549,  553,  556,  560,  564,  568,  571,  575,  579,  582,  586,  589,  593,  596,  600,
	 603,  607,  610,  613,  617,  620,  623,  627,  630,  633,  636,  640,  643,  646,  649,  652,
	 655,  658,  661,  665,  668,  671,  674,  677,  680,  683,  686,  688,  691,  694,  697,  700,
	 703,  706,  709,  711,  714,  717,  720,  723,  725,  728,  731,  733,  736,  739,  742,  744,
	 747,  750,  752,  755,  757,  760,  763,  765,  768,  770,  773,  775,  778,  780,  783,  785,
	 788,  790,  793,  795,  798,  800,  803,  805,  808,  810,  812,  815,  817,  820,  822,  824,
	 827,  829,  831,  834,  836,  838,  841,  843,  845,  847,  850,  852,  854,  856,  859,  861,
	 863,  865,  868,  870,  872,  874,  876,  879,  881,  883,  885,  887,  889,  892,  894,  896,
	 898,  900,  902,  904,  906,  909,  911,  913,  915,  917,  919,  921,  923,  925,  927,  929,
	 931,  933,  935,  937,  939,  941,  943,  945,  947,  949,  951,  953,  955,  957,  959,  961,
	 963,  965,  967,  969,  971,  973,  975,  977,  979,  981,  982,  984,  986,  988,  990,  992,
	 994,  996,  998,  999, 1001, 1003, 1005, 1007, 1009, 1011, 1012, 1014, 1016, 1018, 1020, 1022,
	},
};

struct tcon_gamma_table_s video_curve_2d2 =
{
	{
	   0,	 0,    0,	 0,    0,	 0,    0,	 0,    1,	 1,    1,	 1,    1,	 1,    2,	 2,
	   2,	 3,    3,	 3,    4,	 4,    5,	 5,    6,	 6,    7,	 7,    8,	 9,    9,	10,
	  11,	11,   12,	13,   14,	15,   15,	16,   17,	18,   19,	20,   21,	22,   23,	25,
	  26,	27,   28,	29,   31,	32,   33,	35,   36,	38,   39,	41,   42,	44,   45,	47,
	  49,	50,   52,	54,   55,	57,   59,	61,   63,	65,   67,	69,   71,	73,   75,	77,
	  79,	82,   84,	86,   88,	91,   93,	95,   98,  100,  103,  105,  108,  110,  113,  116,
	 118,  121,  124,  127,  130,  132,  135,  138,  141,  144,  147,  150,  154,  157,  160,  163,
	 166,  170,  173,  176,  180,  183,  187,  190,  194,  197,  201,  204,  208,  212,  216,  219,
	 223,  227,  231,  235,  239,  243,  247,  251,  255,  259,  263,  267,  272,  276,  280,  285,
	 289,  294,  298,  303,  307,  312,  316,  321,  326,  330,  335,  340,  345,  350,  355,  360,
	 365,  370,  375,  380,  385,  390,  395,  401,  406,  411,  417,  422,  427,  433,  438,  444,
	 450,  455,  461,  467,  472,  478,  484,  490,  496,  502,  508,  514,  520,  526,  532,  538,
	 544,  551,  557,  563,  570,  576,  583,  589,  596,  602,  609,  615,  622,  629,  636,  642,
	 649,  656,  663,  670,  677,  684,  691,  698,  705,  713,  720,  727,  735,  742,  749,  757,
	 764,  772,  779,  787,  795,  802,  810,  818,  826,  833,  841,  849,  857,  865,  873,  881,
	 889,  898,  906,  914,  922,  931,  939,  948,  956,  965,  973,  982,  990,  999, 1008, 1016,
	},
};


#endif

