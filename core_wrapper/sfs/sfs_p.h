/**
 *    Copyright (C) 2013 SAS~Storebror <mike@sas1946.com>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SFSH
#define SFSH

namespace
{


const unsigned int FPaTable[] =
{
  0x00000000, 0x23788D5E, 0x46F11ABC, 0x658997E2, 0x0DE23578, 0x2E9AB826,
  0x4B132FC4, 0x686BA29A, 0x38BCE7AE, 0x1BC46AF0, 0x7E4DFD12, 0x5D35704C,
  0x355ED2D6, 0x16265F88, 0x73AFC86A, 0x50D74534, 0x7179CF5C, 0x52014202,
  0x3788D5E0, 0x14F058BE, 0x7C9BFA24, 0x5FE3777A, 0x3A6AE098, 0x19126DC6,
  0x49C528F2, 0x6ABDA5AC, 0x0F34324E, 0x2C4CBF10, 0x44271D8A, 0x675F90D4,
  0x02D60736, 0x21AE8A68, 0x62F39EB8, 0x418B13E6, 0x24028404, 0x077A095A,
  0x6F11ABC0, 0x4C69269E, 0x29E0B17C, 0x0A983C22, 0x5A4F7916, 0x7937F448,
  0x1CBE63AA, 0x3FC6EEF4, 0x57AD4C6E, 0x74D5C130, 0x115C56D2, 0x3224DB8C,
  0x138A51E4, 0x30F2DCBA, 0x557B4B58, 0x7603C606, 0x1E68649C, 0x3D10E9C2,
  0x58997E20, 0x7BE1F37E, 0x2B36B64A, 0x084E3B14, 0x6DC7ACF6, 0x4EBF21A8,
  0x26D48332, 0x05AC0E6C, 0x6025998E, 0x435D14D0, 0x669FB02E, 0x45E73D70,
  0x206EAA92, 0x031627CC, 0x6B7D8556, 0x48050808, 0x2D8C9FEA, 0x0EF412B4,
  0x5E235780, 0x7D5BDADE, 0x18D24D3C, 0x3BAAC062, 0x53C162F8, 0x70B9EFA6,
  0x15307844, 0x3648F51A, 0x17E67F72, 0x349EF22C, 0x511765CE, 0x726FE890,
  0x1A044A0A, 0x397CC754, 0x5CF550B6, 0x7F8DDDE8, 0x2F5A98DC, 0x0C221582,
  0x69AB8260, 0x4AD30F3E, 0x22B8ADA4, 0x01C020FA, 0x6449B718, 0x47313A46,
  0x046C2E96, 0x2714A3C8, 0x429D342A, 0x61E5B974, 0x098E1BEE, 0x2AF696B0,
  0x4F7F0152, 0x6C078C0C, 0x3CD0C938, 0x1FA84466, 0x7A21D384, 0x59595EDA,
  0x3132FC40, 0x124A711E, 0x77C3E6FC, 0x54BB6BA2, 0x7515E1CA, 0x566D6C94,
  0x33E4FB76, 0x109C7628, 0x78F7D4B2, 0x5B8F59EC, 0x3E06CE0E, 0x1D7E4350,
  0x4DA90664, 0x6ED18B3A, 0x0B581CD8, 0x28209186, 0x404B331C, 0x6333BE42,
  0x06BA29A0, 0x25C2A4FE, 0x6E47ED02, 0x4D3F605C, 0x28B6F7BE, 0x0BCE7AE0,
  0x63A5D87A, 0x40DD5524, 0x2554C2C6, 0x062C4F98, 0x56FB0AAC, 0x758387F2,
  0x100A1010, 0x33729D4E, 0x5B193FD4, 0x7861B28A, 0x1DE82568, 0x3E90A836,
  0x1F3E225E, 0x3C46AF00, 0x59CF38E2, 0x7AB7B5BC, 0x12DC1726, 0x31A49A78,
  0x542D0D9A, 0x775580C4, 0x2782C5F0, 0x04FA48AE, 0x6173DF4C, 0x420B5212,
  0x2A60F088, 0x09187DD6, 0x6C91EA34, 0x4FE9676A, 0x0CB473BA, 0x2FCCFEE4,
  0x4A456906, 0x693DE458, 0x015646C2, 0x222ECB9C, 0x47A75C7E, 0x64DFD120,
  0x34089414, 0x1770194A, 0x72F98EA8, 0x518103F6, 0x39EAA16C, 0x1A922C32,
  0x7F1BBBD0, 0x5C63368E, 0x7DCDBCE6, 0x5EB531B8, 0x3B3CA65A, 0x18442B04,
  0x702F899E, 0x535704C0, 0x36DE9322, 0x15A61E7C, 0x45715B48, 0x6609D616,
  0x038041F4, 0x20F8CCAA, 0x48936E30, 0x6BEBE36E, 0x0E62748C, 0x2D1AF9D2,
  0x08D85D2C, 0x2BA0D072, 0x4E294790, 0x6D51CACE, 0x053A6854, 0x2642E50A,
  0x43CB72E8, 0x60B3FFB6, 0x3064BA82, 0x131C37DC, 0x7695A03E, 0x55ED2D60,
  0x3D868FFA, 0x1EFE02A4, 0x7B779546, 0x580F1818, 0x79A19270, 0x5AD91F2E,
  0x3F5088CC, 0x1C280592, 0x7443A708, 0x573B2A56, 0x32B2BDB4, 0x11CA30EA,
  0x411D75DE, 0x6265F880, 0x07EC6F62, 0x2494E23C, 0x4CFF40A6, 0x6F87CDF8,
  0x0A0E5A1A, 0x2976D744, 0x6A2BC394, 0x49534ECA, 0x2CDAD928, 0x0FA25476,
  0x67C9F6EC, 0x44B17BB2, 0x2138EC50, 0x0240610E, 0x5297243A, 0x71EFA964,
  0x14663E86, 0x371EB3D8, 0x5F751142, 0x7C0D9C1C, 0x19840BFE, 0x3AFC86A0,
  0x1B520CC8, 0x382A8196, 0x5DA31674, 0x7EDB9B2A, 0x16B039B0, 0x35C8B4EE,
  0x5041230C, 0x7339AE52, 0x23EEEB66, 0x00966638, 0x651FF1DA, 0x46677C84,
  0x2E0CDE1E, 0x0D745340, 0x68FDC4A2, 0x4B8549FC
};

const unsigned int FPbTable[] =
{
  0x00000000, 0x1434182E, 0x3C5C2872, 0x2868305C, 0x6C8C48CA, 0x78B850E4,
  0x50D060B8, 0x44E47896, 0x4D2C89BA, 0x59189194, 0x7170A1C8, 0x6544B9E6,
  0x21A0C170, 0x3594D95E, 0x1DFCE902, 0x09C8F12C, 0x1A591374, 0x0E6D0B5A,
  0x26053B06, 0x32312328, 0x76D55BBE, 0x62E14390, 0x4A8973CC, 0x5EBD6BE2,
  0x57759ACE, 0x434182E0, 0x6B29B2BC, 0x7F1DAA92, 0x3BF9D204, 0x2FCDCA2A,
  0x07A5FA76, 0x1391E258, 0x20863EC6, 0x34B226E8, 0x1CDA16B4, 0x08EE0E9A,
  0x4C0A760C, 0x583E6E22, 0x70565E7E, 0x64624650, 0x6DAAB77C, 0x799EAF52,
  0x51F69F0E, 0x45C28720, 0x0126FFB6, 0x1512E798, 0x3D7AD7C4, 0x294ECFEA,
  0x3ADF2DB2, 0x2EEB359C, 0x068305C0, 0x12B71DEE, 0x56536578, 0x42677D56,
  0x6A0F4D0A, 0x7E3B5524, 0x77F3A408, 0x63C7BC26, 0x4BAF8C7A, 0x5F9B9454,
  0x1B7FECC2, 0x0F4BF4EC, 0x2723C4B0, 0x3317DC9E, 0x553865A2, 0x410C7D8C,
  0x69644DD0, 0x7D5055FE, 0x39B42D68, 0x2D803546, 0x05E8051A, 0x11DC1D34,
  0x1814EC18, 0x0C20F436, 0x2448C46A, 0x307CDC44, 0x7498A4D2, 0x60ACBCFC,
  0x48C48CA0, 0x5CF0948E, 0x4F6176D6, 0x5B556EF8, 0x733D5EA4, 0x6709468A,
  0x23ED3E1C, 0x37D92632, 0x1FB1166E, 0x0B850E40, 0x024DFF6C, 0x1679E742,
  0x3E11D71E, 0x2A25CF30, 0x6EC1B7A6, 0x7AF5AF88, 0x529D9FD4, 0x46A987FA,
  0x75BE5B64, 0x618A434A, 0x49E27316, 0x5DD66B38, 0x193213AE, 0x0D060B80,
  0x256E3BDC, 0x315A23F2, 0x3892D2DE, 0x2CA6CAF0, 0x04CEFAAC, 0x10FAE282,
  0x541E9A14, 0x402A823A, 0x6842B266, 0x7C76AA48, 0x6FE74810, 0x7BD3503E,
  0x53BB6062, 0x478F784C, 0x036B00DA, 0x175F18F4, 0x3F3728A8, 0x2B033086,
  0x22CBC1AA, 0x36FFD984, 0x1E97E9D8, 0x0AA3F1F6, 0x4E478960, 0x5A73914E,
  0x721BA112, 0x662FB93C, 0x3E44D36A, 0x2A70CB44, 0x0218FB18, 0x162CE336,
  0x52C89BA0, 0x46FC838E, 0x6E94B3D2, 0x7AA0ABFC, 0x73685AD0, 0x675C42FE,
  0x4F3472A2, 0x5B006A8C, 0x1FE4121A, 0x0BD00A34, 0x23B83A68, 0x378C2246,
  0x241DC01E, 0x3029D830, 0x1841E86C, 0x0C75F042, 0x489188D4, 0x5CA590FA,
  0x74CDA0A6, 0x60F9B888, 0x693149A4, 0x7D05518A, 0x556D61D6, 0x415979F8,
  0x05BD016E, 0x11891940, 0x39E1291C, 0x2DD53132, 0x1EC2EDAC, 0x0AF6F582,
  0x229EC5DE, 0x36AADDF0, 0x724EA566, 0x667ABD48, 0x4E128D14, 0x5A26953A,
  0x53EE6416, 0x47DA7C38, 0x6FB24C64, 0x7B86544A, 0x3F622CDC, 0x2B5634F2,
  0x033E04AE, 0x170A1C80, 0x049BFED8, 0x10AFE6F6, 0x38C7D6AA, 0x2CF3CE84,
  0x6817B612, 0x7C23AE3C, 0x544B9E60, 0x407F864E, 0x49B77762, 0x5D836F4C,
  0x75EB5F10, 0x61DF473E, 0x253B3FA8, 0x310F2786, 0x196717DA, 0x0D530FF4,
  0x6B7CB6C8, 0x7F48AEE6, 0x57209EBA, 0x43148694, 0x07F0FE02, 0x13C4E62C,
  0x3BACD670, 0x2F98CE5E, 0x26503F72, 0x3264275C, 0x1A0C1700, 0x0E380F2E,
  0x4ADC77B8, 0x5EE86F96, 0x76805FCA, 0x62B447E4, 0x7125A5BC, 0x6511BD92,
  0x4D798DCE, 0x594D95E0, 0x1DA9ED76, 0x099DF558, 0x21F5C504, 0x35C1DD2A,
  0x3C092C06, 0x283D3428, 0x00550474, 0x14611C5A, 0x508564CC, 0x44B17CE2,
  0x6CD94CBE, 0x78ED5490, 0x4BFA880E, 0x5FCE9020, 0x77A6A07C, 0x6392B852,
  0x2776C0C4, 0x3342D8EA, 0x1B2AE8B6, 0x0F1EF098, 0x06D601B4, 0x12E2199A,
  0x3A8A29C6, 0x2EBE31E8, 0x6A5A497E, 0x7E6E5150, 0x5606610C, 0x42327922,
  0x51A39B7A, 0x45978354, 0x6DFFB308, 0x79CBAB26, 0x3D2FD3B0, 0x291BCB9E,
  0x0173FBC2, 0x1547E3EC, 0x1C8F12C0, 0x08BB0AEE, 0x20D33AB2, 0x34E7229C,
  0x70035A0A, 0x64374224, 0x4C5F7278, 0x586B6A56
};


unsigned __int64 makeHash(const unsigned __int64 hash, const void *buf,
                          const int len)
{
  unsigned char c;
  unsigned a = (unsigned)(hash & 0xFFFFFFFF);
  unsigned b = (unsigned)(hash >> 32 & 0xFFFFFFFF);
  for(int i = 0; i < len; i++)
  {
    c = ((unsigned char *)buf)[i];
    a = (a << 8 | c) ^ FPaTable[a >> 24];
    b = (b << 8 | c) ^ FPbTable[b >> 24];
  }
  return ((unsigned __int64)a & 0xFFFFFFFF) | (unsigned __int64)b << 32;
}


} // namespace

#endif
