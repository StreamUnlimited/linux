// SPDX-License-Identifier: GPL-2.0-only
/*
* Realtek wireless local area network IC driver.
*   This is an interface between cfg80211 and firmware in other core. The
*   commnunication between driver and firmware is IPC（Inter Process
*   Communication）bus.
*
* Copyright (C) 2023, Realtek Corporation. All rights reserved.
*/

#include <rtw_cfg80211_fullmac.h>

static const struct ieee80211_regdomain rtl_regdom_11 = {
	.n_reg_rules = 1,
	.alpha2 = "99",
	.reg_rules = {
		RTL819x_2GHZ_CH01_11,
	}
};

static const struct ieee80211_regdomain rtl_regdom_12_13 = {
	.n_reg_rules = 2,
	.alpha2 = "99",
	.reg_rules = {
		RTL819x_2GHZ_CH01_11,
		RTL819x_2GHZ_CH12_13,
	}
};

static const struct ieee80211_regdomain rtl_regdom_no_midband = {
	.n_reg_rules = 3,
	.alpha2 = "99",
	.reg_rules = {
		RTL819x_2GHZ_CH01_11,
		RTL819x_5GHZ_5150_5350,
		RTL819x_5GHZ_5725_5850,
	}
};

static const struct ieee80211_regdomain rtl_regdom_60_64 = {
	.n_reg_rules = 3,
	.alpha2 = "99",
	.reg_rules = {
		RTL819x_2GHZ_CH01_11,
		RTL819x_2GHZ_CH12_13,
		RTL819x_5GHZ_5725_5850,
	}
};

static const struct ieee80211_regdomain rtl_regdom_14_60_64 = {
	.n_reg_rules = 4,
	.alpha2 = "99",
	.reg_rules = {
		RTL819x_2GHZ_CH01_11,
		RTL819x_2GHZ_CH12_13,
		RTL819x_2GHZ_CH14,
		RTL819x_5GHZ_5725_5850,
	}
};

static const struct ieee80211_regdomain rtl_regdom_12_13_5g_all = {
	.n_reg_rules = 4,
	.alpha2 = "99",
	.reg_rules = {
		RTL819x_2GHZ_CH01_11,
		RTL819x_2GHZ_CH12_13,
		RTL819x_5GHZ_5150_5350,
		RTL819x_5GHZ_5470_5850,
	}
};

static const struct ieee80211_regdomain rtl_regdom_14 = {
	.n_reg_rules = 3,
	.alpha2 = "99",
	.reg_rules = {
		RTL819x_2GHZ_CH01_11,
		RTL819x_2GHZ_CH12_13,
		RTL819x_2GHZ_CH14,
	}
};

static const struct country_chplan country_chplan_map[] = {
	COUNTRY_CHPLAN_ENT("AD", 0x3A, TXPWR_LMT_ETSI),     /* Andorra */
	COUNTRY_CHPLAN_ENT("AE", 0x5E, TXPWR_LMT_ETSI),     /* United Arab Emirates */
	COUNTRY_CHPLAN_ENT("AF", 0x42, TXPWR_LMT_ETSI),     /* Afghanistan */
	COUNTRY_CHPLAN_ENT("AG", 0x76, TXPWR_LMT_FCC),     /* Antigua & Barbuda */
	COUNTRY_CHPLAN_ENT("AI", 0x5E, TXPWR_LMT_ETSI),     /* Anguilla(UK) */
	COUNTRY_CHPLAN_ENT("AL", 0x5E, TXPWR_LMT_ETSI),     /* Albania */
	COUNTRY_CHPLAN_ENT("AM", 0x5E, TXPWR_LMT_ETSI),     /* Armenia */
	COUNTRY_CHPLAN_ENT("AN", 0x76, TXPWR_LMT_FCC),     /* Netherlands Antilles */
	COUNTRY_CHPLAN_ENT("AO", 0x5E, TXPWR_LMT_ETSI),     /* Angola */
	COUNTRY_CHPLAN_ENT("AQ", 0x26, TXPWR_LMT_ETSI),     /* Antarctica */
	COUNTRY_CHPLAN_ENT("AR", 0x4D, TXPWR_LMT_MEXICO),     /* Argentina */
	COUNTRY_CHPLAN_ENT("AS", 0x76, TXPWR_LMT_FCC),     /* American Samoa */
	COUNTRY_CHPLAN_ENT("AT", 0x5E, TXPWR_LMT_ETSI),     /* Austria */
	COUNTRY_CHPLAN_ENT("AU", 0x03, TXPWR_LMT_ACMA),     /* Australia */
	COUNTRY_CHPLAN_ENT("AW", 0x76, TXPWR_LMT_FCC),     /* Aruba */
	COUNTRY_CHPLAN_ENT("AZ", 0x5E, TXPWR_LMT_ETSI),     /* Azerbaijan */
	COUNTRY_CHPLAN_ENT("BA", 0x5E, TXPWR_LMT_ETSI),     /* Bosnia & Herzegovina */
	COUNTRY_CHPLAN_ENT("BB", 0x76, TXPWR_LMT_FCC),     /* Barbados */
	COUNTRY_CHPLAN_ENT("BD", 0x5E, TXPWR_LMT_ETSI),     /* Bangladesh */
	COUNTRY_CHPLAN_ENT("BE", 0x5E, TXPWR_LMT_ETSI),     /* Belgium */
	COUNTRY_CHPLAN_ENT("BF", 0x5E, TXPWR_LMT_ETSI),     /* Burkina Faso */
	COUNTRY_CHPLAN_ENT("BG", 0x5E, TXPWR_LMT_ETSI),     /* Bulgaria */
	COUNTRY_CHPLAN_ENT("BH", 0x06, TXPWR_LMT_ETSI),     /* Bahrain */
	COUNTRY_CHPLAN_ENT("BI", 0x3A, TXPWR_LMT_ETSI),     /* Burundi */
	COUNTRY_CHPLAN_ENT("BJ", 0x3A, TXPWR_LMT_ETSI),     /* Benin */
	COUNTRY_CHPLAN_ENT("BM", 0x76, TXPWR_LMT_FCC),     /* Bermuda (UK) */
	COUNTRY_CHPLAN_ENT("BN", 0x06, TXPWR_LMT_ETSI),     /* Brunei */
	COUNTRY_CHPLAN_ENT("BO", 0x11, TXPWR_LMT_FCC),     /* Bolivia */
	COUNTRY_CHPLAN_ENT("BR", 0x62, TXPWR_LMT_FCC),     /* Brazil */
	COUNTRY_CHPLAN_ENT("BS", 0x76, TXPWR_LMT_FCC),     /* Bahamas */
	COUNTRY_CHPLAN_ENT("BT", 0x5E, TXPWR_LMT_ETSI),     /* Bhutan */
	COUNTRY_CHPLAN_ENT("BV", 0x5E, TXPWR_LMT_ETSI),     /* Bouvet Island (Norway) */
	COUNTRY_CHPLAN_ENT("BW", 0x5E, TXPWR_LMT_ETSI),     /* Botswana */
	COUNTRY_CHPLAN_ENT("BY", 0x08, TXPWR_LMT_ETSI),     /* Belarus */
	COUNTRY_CHPLAN_ENT("BZ", 0x76, TXPWR_LMT_FCC),     /* Belize */
	COUNTRY_CHPLAN_ENT("CA", 0x10, TXPWR_LMT_IC),     /* Canada */
	COUNTRY_CHPLAN_ENT("CC", 0x03, TXPWR_LMT_ACMA),     /* Cocos (Keeling) Islands (Australia) */
	COUNTRY_CHPLAN_ENT("CD", 0x5E, TXPWR_LMT_ETSI),     /* Congo, Republic of the */
	COUNTRY_CHPLAN_ENT("CF", 0x3A, TXPWR_LMT_ETSI),     /* Central African Republic */
	COUNTRY_CHPLAN_ENT("CG", 0x5E, TXPWR_LMT_ETSI),     /* Congo, Democratic Republic of the. Zaire */
	COUNTRY_CHPLAN_ENT("CH", 0x5E, TXPWR_LMT_ETSI),     /* Switzerland */
	COUNTRY_CHPLAN_ENT("CI", 0x42, TXPWR_LMT_ETSI),     /* Cote d'Ivoire */
	COUNTRY_CHPLAN_ENT("CK", 0x5E, TXPWR_LMT_ETSI),     /* Cook Islands */
	COUNTRY_CHPLAN_ENT("CL", 0x76, TXPWR_LMT_CHILE),     /* Chile */
	COUNTRY_CHPLAN_ENT("CM", 0x5E, TXPWR_LMT_ETSI),     /* Cameroon */
	COUNTRY_CHPLAN_ENT("CN", 0x06, TXPWR_LMT_CN),     /* China */
	COUNTRY_CHPLAN_ENT("CO", 0x76, TXPWR_LMT_FCC),     /* Colombia */
	COUNTRY_CHPLAN_ENT("CR", 0x76, TXPWR_LMT_FCC),     /* Costa Rica */
	COUNTRY_CHPLAN_ENT("CV", 0x5E, TXPWR_LMT_ETSI),     /* Cape Verde */
	COUNTRY_CHPLAN_ENT("CX", 0x03, TXPWR_LMT_ACMA),     /* Christmas Island (Australia) */
	COUNTRY_CHPLAN_ENT("CY", 0x5E, TXPWR_LMT_ETSI),     /* Cyprus */
	COUNTRY_CHPLAN_ENT("CZ", 0x5E, TXPWR_LMT_ETSI),     /* Czech Republic */
	COUNTRY_CHPLAN_ENT("DE", 0x5E, TXPWR_LMT_ETSI),     /* Germany */
	COUNTRY_CHPLAN_ENT("DJ", 0x5E, TXPWR_LMT_ETSI),     /* Djibouti */
	COUNTRY_CHPLAN_ENT("DK", 0x5E, TXPWR_LMT_ETSI),     /* Denmark */
	COUNTRY_CHPLAN_ENT("DM", 0x76, TXPWR_LMT_FCC),     /* Dominica */
	COUNTRY_CHPLAN_ENT("DO", 0x76, TXPWR_LMT_FCC),     /* Dominican Republic */
	COUNTRY_CHPLAN_ENT("DZ", 0x00, TXPWR_LMT_ETSI),     /* Algeria */
	COUNTRY_CHPLAN_ENT("EC", 0x76, TXPWR_LMT_FCC),     /* Ecuador */
	COUNTRY_CHPLAN_ENT("EE", 0x5E, TXPWR_LMT_ETSI),     /* Estonia */
	COUNTRY_CHPLAN_ENT("EG", 0x3C, TXPWR_LMT_ETSI),     /* Egypt */
	COUNTRY_CHPLAN_ENT("EH", 0x3C, TXPWR_LMT_ETSI),     /* Western Sahara */
	COUNTRY_CHPLAN_ENT("ER", 0x3A, TXPWR_LMT_ETSI),     /* Eritrea */
	COUNTRY_CHPLAN_ENT("ES", 0x5E, TXPWR_LMT_ETSI),     /* Spain, Canary Islands, Ceuta, Melilla */
	COUNTRY_CHPLAN_ENT("ET", 0x3A, TXPWR_LMT_ETSI),     /* Ethiopia */
	COUNTRY_CHPLAN_ENT("FI", 0x5E, TXPWR_LMT_ETSI),     /* Finland */
	COUNTRY_CHPLAN_ENT("FJ", 0x76, TXPWR_LMT_FCC),     /* Fiji */
	COUNTRY_CHPLAN_ENT("FK", 0x5E, TXPWR_LMT_ETSI),     /* Falkland Islands (Islas Malvinas) (UK) */
	COUNTRY_CHPLAN_ENT("FM", 0x76, TXPWR_LMT_FCC),     /* Micronesia, Federated States of (USA) */
	COUNTRY_CHPLAN_ENT("FO", 0x5E, TXPWR_LMT_ETSI),     /* Faroe Islands (Denmark) */
	COUNTRY_CHPLAN_ENT("FR", 0x5E, TXPWR_LMT_ETSI),     /* France */
	COUNTRY_CHPLAN_ENT("GA", 0x5E, TXPWR_LMT_ETSI),     /* Gabon */
	COUNTRY_CHPLAN_ENT("GB", 0x0B, TXPWR_LMT_UK),     /* Great Britain (United Kingdom; England) */
	COUNTRY_CHPLAN_ENT("GD", 0x76, TXPWR_LMT_FCC),     /* Grenada */
	COUNTRY_CHPLAN_ENT("GE", 0x5E, TXPWR_LMT_ETSI),     /* Georgia */
	COUNTRY_CHPLAN_ENT("GF", 0x3A, TXPWR_LMT_ETSI),     /* French Guiana */
	COUNTRY_CHPLAN_ENT("GG", 0x5E, TXPWR_LMT_ETSI),     /* Guernsey (UK) */
	COUNTRY_CHPLAN_ENT("GH", 0x5E, TXPWR_LMT_ETSI),     /* Ghana */
	COUNTRY_CHPLAN_ENT("GI", 0x5E, TXPWR_LMT_ETSI),     /* Gibraltar (UK) */
	COUNTRY_CHPLAN_ENT("GL", 0x5E, TXPWR_LMT_ETSI),     /* Greenland (Denmark) */
	COUNTRY_CHPLAN_ENT("GM", 0x5E, TXPWR_LMT_ETSI),     /* Gambia */
	COUNTRY_CHPLAN_ENT("GN", 0x5E, TXPWR_LMT_ETSI),     /* Guinea */
	COUNTRY_CHPLAN_ENT("GP", 0x5E, TXPWR_LMT_ETSI),     /* Guadeloupe (France) */
	COUNTRY_CHPLAN_ENT("GQ", 0x5E, TXPWR_LMT_ETSI),     /* Equatorial Guinea */
	COUNTRY_CHPLAN_ENT("GR", 0x5E, TXPWR_LMT_ETSI),     /* Greece */
	COUNTRY_CHPLAN_ENT("GS", 0x3A, TXPWR_LMT_ETSI),     /* South Georgia and the Sandwich Islands (UK) */
	COUNTRY_CHPLAN_ENT("GT", 0x76, TXPWR_LMT_FCC),     /* Guatemala */
	COUNTRY_CHPLAN_ENT("GU", 0x76, TXPWR_LMT_FCC),     /* Guam (USA) */
	COUNTRY_CHPLAN_ENT("GW", 0x5E, TXPWR_LMT_ETSI),     /* Guinea-Bissau */
	COUNTRY_CHPLAN_ENT("GY", 0x76, TXPWR_LMT_FCC),     /* Guyana */
	COUNTRY_CHPLAN_ENT("HK", 0x5E, TXPWR_LMT_ETSI),     /* Hong Kong */
	COUNTRY_CHPLAN_ENT("HM", 0x03, TXPWR_LMT_ACMA),     /* Heard and McDonald Islands (Australia) */
	COUNTRY_CHPLAN_ENT("HN", 0x76, TXPWR_LMT_FCC),     /* Honduras */
	COUNTRY_CHPLAN_ENT("HR", 0x5E, TXPWR_LMT_ETSI),     /* Croatia */
	COUNTRY_CHPLAN_ENT("HT", 0x76, TXPWR_LMT_FCC),     /* Haiti */
	COUNTRY_CHPLAN_ENT("HU", 0x5E, TXPWR_LMT_ETSI),     /* Hungary */
	COUNTRY_CHPLAN_ENT("ID", 0x5D, TXPWR_LMT_ETSI),     /* Indonesia */
	COUNTRY_CHPLAN_ENT("IE", 0x5E, TXPWR_LMT_ETSI),     /* Ireland */
	COUNTRY_CHPLAN_ENT("IL", 0x5E, TXPWR_LMT_ETSI),     /* Israel */
	COUNTRY_CHPLAN_ENT("IM", 0x5E, TXPWR_LMT_ETSI),     /* Isle of Man (UK) */
	COUNTRY_CHPLAN_ENT("IN", 0x5E, TXPWR_LMT_ETSI),     /* India */
	COUNTRY_CHPLAN_ENT("IO", 0x3A, TXPWR_LMT_ETSI),     /* British Indian Ocean Territory (UK) */
	COUNTRY_CHPLAN_ENT("IQ", 0x05, TXPWR_LMT_ETSI),     /* Iraq */
	COUNTRY_CHPLAN_ENT("IR", 0x3A, TXPWR_LMT_ETSI),     /* Iran */
	COUNTRY_CHPLAN_ENT("IS", 0x5E, TXPWR_LMT_ETSI),     /* Iceland */
	COUNTRY_CHPLAN_ENT("IT", 0x5E, TXPWR_LMT_ETSI),     /* Italy */
	COUNTRY_CHPLAN_ENT("JE", 0x5E, TXPWR_LMT_ETSI),     /* Jersey (UK) */
	COUNTRY_CHPLAN_ENT("JM", 0x76, TXPWR_LMT_FCC),     /* Jamaica */
	COUNTRY_CHPLAN_ENT("JO", 0x5E, TXPWR_LMT_ETSI),     /* Jordan */
	COUNTRY_CHPLAN_ENT("JP", 0x7D, TXPWR_LMT_MKK),     /* Japan- Telec */
	COUNTRY_CHPLAN_ENT("KE", 0x5E, TXPWR_LMT_ETSI),     /* Kenya */
	COUNTRY_CHPLAN_ENT("KG", 0x5E, TXPWR_LMT_ETSI),     /* Kyrgyzstan */
	COUNTRY_CHPLAN_ENT("KH", 0x5E, TXPWR_LMT_ETSI),     /* Cambodia */
	COUNTRY_CHPLAN_ENT("KI", 0x5E, TXPWR_LMT_ETSI),     /* Kiribati */
	COUNTRY_CHPLAN_ENT("KM", 0x5E, TXPWR_LMT_ETSI),     /* Comoros */
	COUNTRY_CHPLAN_ENT("KN", 0x76, TXPWR_LMT_FCC),     /* Saint Kitts and Nevis */
	COUNTRY_CHPLAN_ENT("KR", 0x4B, TXPWR_LMT_KCC),     /* South Korea */
	COUNTRY_CHPLAN_ENT("KW", 0x5E, TXPWR_LMT_ETSI),     /* Kuwait */
	COUNTRY_CHPLAN_ENT("KY", 0x76, TXPWR_LMT_FCC),     /* Cayman Islands (UK) */
	COUNTRY_CHPLAN_ENT("KZ", 0x5E, TXPWR_LMT_ETSI),     /* Kazakhstan */
	COUNTRY_CHPLAN_ENT("LA", 0x5E, TXPWR_LMT_ETSI),     /* Laos */
	COUNTRY_CHPLAN_ENT("LB", 0x5E, TXPWR_LMT_ETSI),     /* Lebanon */
	COUNTRY_CHPLAN_ENT("LC", 0x76, TXPWR_LMT_FCC),     /* Saint Lucia */
	COUNTRY_CHPLAN_ENT("LI", 0x5E, TXPWR_LMT_ETSI),     /* Liechtenstein */
	COUNTRY_CHPLAN_ENT("LK", 0x5E, TXPWR_LMT_ETSI),     /* Sri Lanka */
	COUNTRY_CHPLAN_ENT("LR", 0x5E, TXPWR_LMT_ETSI),     /* Liberia */
	COUNTRY_CHPLAN_ENT("LS", 0x5E, TXPWR_LMT_ETSI),     /* Lesotho */
	COUNTRY_CHPLAN_ENT("LT", 0x5E, TXPWR_LMT_ETSI),     /* Lithuania */
	COUNTRY_CHPLAN_ENT("LU", 0x5E, TXPWR_LMT_ETSI),     /* Luxembourg */
	COUNTRY_CHPLAN_ENT("LV", 0x5E, TXPWR_LMT_ETSI),     /* Latvia */
	COUNTRY_CHPLAN_ENT("LY", 0x5E, TXPWR_LMT_ETSI),     /* Libya */
	COUNTRY_CHPLAN_ENT("MA", 0x3C, TXPWR_LMT_ETSI),     /* Morocco */
	COUNTRY_CHPLAN_ENT("MC", 0x5E, TXPWR_LMT_ETSI),     /* Monaco */
	COUNTRY_CHPLAN_ENT("MD", 0x5E, TXPWR_LMT_ETSI),     /* Moldova */
	COUNTRY_CHPLAN_ENT("ME", 0x3A, TXPWR_LMT_ETSI),     /* Montenegro */
	COUNTRY_CHPLAN_ENT("MF", 0x76, TXPWR_LMT_FCC),     /* Saint Martin */
	COUNTRY_CHPLAN_ENT("MG", 0x5E, TXPWR_LMT_ETSI),     /* Madagascar */
	COUNTRY_CHPLAN_ENT("MH", 0x76, TXPWR_LMT_FCC),     /* Marshall Islands (USA) */
	COUNTRY_CHPLAN_ENT("MK", 0x5E, TXPWR_LMT_ETSI),     /* Republic of Macedonia (FYROM) */
	COUNTRY_CHPLAN_ENT("ML", 0x5E, TXPWR_LMT_ETSI),     /* Mali */
	COUNTRY_CHPLAN_ENT("MM", 0x3A, TXPWR_LMT_ETSI),     /* Burma (Myanmar) */
	COUNTRY_CHPLAN_ENT("MN", 0x5E, TXPWR_LMT_ETSI),     /* Mongolia */
	COUNTRY_CHPLAN_ENT("MO", 0x5E, TXPWR_LMT_ETSI),     /* Macau */
	COUNTRY_CHPLAN_ENT("MP", 0x76, TXPWR_LMT_FCC),     /* Northern Mariana Islands (USA) */
	COUNTRY_CHPLAN_ENT("MQ", 0x3A, TXPWR_LMT_ETSI),     /* Martinique (France) */
	COUNTRY_CHPLAN_ENT("MR", 0x5E, TXPWR_LMT_ETSI),     /* Mauritania */
	COUNTRY_CHPLAN_ENT("MS", 0x3A, TXPWR_LMT_ETSI),     /* Montserrat (UK) */
	COUNTRY_CHPLAN_ENT("MT", 0x5E, TXPWR_LMT_ETSI),     /* Malta */
	COUNTRY_CHPLAN_ENT("MU", 0x5E, TXPWR_LMT_ETSI),     /* Mauritius */
	COUNTRY_CHPLAN_ENT("MV", 0x3C, TXPWR_LMT_ETSI),     /* Maldives */
	COUNTRY_CHPLAN_ENT("MW", 0x5E, TXPWR_LMT_ETSI),     /* Malawi */
	COUNTRY_CHPLAN_ENT("MX", 0x4D, TXPWR_LMT_MEXICO),     /* Mexico */
	COUNTRY_CHPLAN_ENT("MY", 0x07, TXPWR_LMT_ETSI),     /* Malaysia */
	COUNTRY_CHPLAN_ENT("MZ", 0x5E, TXPWR_LMT_ETSI),     /* Mozambique */
	COUNTRY_CHPLAN_ENT("NA", 0x5E, TXPWR_LMT_ETSI),     /* Namibia */
	COUNTRY_CHPLAN_ENT("NC", 0x3A, TXPWR_LMT_ETSI),     /* New Caledonia */
	COUNTRY_CHPLAN_ENT("NE", 0x5E, TXPWR_LMT_ETSI),     /* Niger */
	COUNTRY_CHPLAN_ENT("NF", 0x03, TXPWR_LMT_ACMA),     /* Norfolk Island (Australia) */
	COUNTRY_CHPLAN_ENT("NG", 0x75, TXPWR_LMT_ETSI),     /* Nigeria */
	COUNTRY_CHPLAN_ENT("NI", 0x76, TXPWR_LMT_FCC),     /* Nicaragua */
	COUNTRY_CHPLAN_ENT("NL", 0x5E, TXPWR_LMT_ETSI),     /* Netherlands */
	COUNTRY_CHPLAN_ENT("NO", 0x5E, TXPWR_LMT_ETSI),     /* Norway */
	COUNTRY_CHPLAN_ENT("NP", 0x06, TXPWR_LMT_ETSI),     /* Nepal */
	COUNTRY_CHPLAN_ENT("NR", 0x5E, TXPWR_LMT_ETSI),     /* Nauru */
	COUNTRY_CHPLAN_ENT("NU", 0x03, TXPWR_LMT_ACMA),     /* Niue */
	COUNTRY_CHPLAN_ENT("NZ", 0x03, TXPWR_LMT_ACMA),     /* New Zealand */
	COUNTRY_CHPLAN_ENT("OM", 0x5E, TXPWR_LMT_ETSI),     /* Oman */
	COUNTRY_CHPLAN_ENT("PA", 0x76, TXPWR_LMT_FCC),     /* Panama */
	COUNTRY_CHPLAN_ENT("PE", 0x76, TXPWR_LMT_FCC),     /* Peru */
	COUNTRY_CHPLAN_ENT("PF", 0x3A, TXPWR_LMT_ETSI),     /* French Polynesia (France) */
	COUNTRY_CHPLAN_ENT("PG", 0x5E, TXPWR_LMT_ETSI),     /* Papua New Guinea */
	COUNTRY_CHPLAN_ENT("PH", 0x5E, TXPWR_LMT_ETSI),     /* Philippines */
	COUNTRY_CHPLAN_ENT("PK", 0x5E, TXPWR_LMT_ETSI),     /* Pakistan */
	COUNTRY_CHPLAN_ENT("PL", 0x5E, TXPWR_LMT_ETSI),     /* Poland */
	COUNTRY_CHPLAN_ENT("PM", 0x5E, TXPWR_LMT_ETSI),     /* Saint Pierre and Miquelon (France) */
	COUNTRY_CHPLAN_ENT("PR", 0x76, TXPWR_LMT_FCC),     /* Puerto Rico */
	COUNTRY_CHPLAN_ENT("PS", 0x5E, TXPWR_LMT_ETSI),     /* Palestine */
	COUNTRY_CHPLAN_ENT("PT", 0x5E, TXPWR_LMT_ETSI),     /* Portugal */
	COUNTRY_CHPLAN_ENT("PW", 0x76, TXPWR_LMT_FCC),     /* Palau */
	COUNTRY_CHPLAN_ENT("PY", 0x76, TXPWR_LMT_FCC),     /* Paraguay */
	COUNTRY_CHPLAN_ENT("QA", 0x5E, TXPWR_LMT_QATAR),     /* Qatar */
	COUNTRY_CHPLAN_ENT("RE", 0x5E, TXPWR_LMT_ETSI),     /* Reunion (France) */
	COUNTRY_CHPLAN_ENT("RO", 0x5E, TXPWR_LMT_ETSI),     /* Romania */
	COUNTRY_CHPLAN_ENT("RS", 0x5E, TXPWR_LMT_ETSI),     /* Serbia */
	COUNTRY_CHPLAN_ENT("RU", 0x09, TXPWR_LMT_ETSI),     /* Russia(fac/gost), Kaliningrad */
	COUNTRY_CHPLAN_ENT("RW", 0x5E, TXPWR_LMT_ETSI),     /* Rwanda */
	COUNTRY_CHPLAN_ENT("SA", 0x5E, TXPWR_LMT_ETSI),     /* Saudi Arabia */
	COUNTRY_CHPLAN_ENT("SB", 0x3A, TXPWR_LMT_ETSI),     /* Solomon Islands */
	COUNTRY_CHPLAN_ENT("SC", 0x76, TXPWR_LMT_FCC),     /* Seychelles */
	COUNTRY_CHPLAN_ENT("SE", 0x5E, TXPWR_LMT_ETSI),     /* Sweden */
	COUNTRY_CHPLAN_ENT("SG", 0x5E, TXPWR_LMT_ETSI),     /* Singapore */
	COUNTRY_CHPLAN_ENT("SH", 0x5E, TXPWR_LMT_ETSI),     /* Saint Helena (UK) */
	COUNTRY_CHPLAN_ENT("SI", 0x5E, TXPWR_LMT_ETSI),     /* Slovenia */
	COUNTRY_CHPLAN_ENT("SJ", 0x5E, TXPWR_LMT_ETSI),     /* Svalbard (Norway) */
	COUNTRY_CHPLAN_ENT("SK", 0x5E, TXPWR_LMT_ETSI),     /* Slovakia */
	COUNTRY_CHPLAN_ENT("SL", 0x5E, TXPWR_LMT_ETSI),     /* Sierra Leone */
	COUNTRY_CHPLAN_ENT("SM", 0x3A, TXPWR_LMT_ETSI),     /* San Marino */
	COUNTRY_CHPLAN_ENT("SN", 0x3A, TXPWR_LMT_ETSI),     /* Senegal */
	COUNTRY_CHPLAN_ENT("SO", 0x3A, TXPWR_LMT_ETSI),     /* Somalia */
	COUNTRY_CHPLAN_ENT("SR", 0x74, TXPWR_LMT_FCC),     /* Suriname */
	COUNTRY_CHPLAN_ENT("ST", 0x5E, TXPWR_LMT_ETSI),     /* Sao Tome and Principe */
	COUNTRY_CHPLAN_ENT("SV", 0x76, TXPWR_LMT_FCC),     /* El Salvador */
	COUNTRY_CHPLAN_ENT("SX", 0x76, TXPWR_LMT_FCC),     /* Sint Marteen */
	COUNTRY_CHPLAN_ENT("SZ", 0x5E, TXPWR_LMT_ETSI),     /* Swaziland */
	COUNTRY_CHPLAN_ENT("TC", 0x3A, TXPWR_LMT_ETSI),     /* Turks and Caicos Islands (UK) */
	COUNTRY_CHPLAN_ENT("TD", 0x3A, TXPWR_LMT_ETSI),     /* Chad */
	COUNTRY_CHPLAN_ENT("TF", 0x3A, TXPWR_LMT_ETSI),     /* French Southern and Antarctic Lands (FR Southern Territories) */
	COUNTRY_CHPLAN_ENT("TG", 0x5E, TXPWR_LMT_ETSI),     /* Togo */
	COUNTRY_CHPLAN_ENT("TH", 0x5E, TXPWR_LMT_ETSI),     /* Thailand */
	COUNTRY_CHPLAN_ENT("TJ", 0x5E, TXPWR_LMT_ETSI),     /* Tajikistan */
	COUNTRY_CHPLAN_ENT("TK", 0x03, TXPWR_LMT_ACMA),     /* Tokelau */
	COUNTRY_CHPLAN_ENT("TM", 0x3A, TXPWR_LMT_ETSI),     /* Turkmenistan */
	COUNTRY_CHPLAN_ENT("TN", 0x04, TXPWR_LMT_ETSI),     /* Tunisia */
	COUNTRY_CHPLAN_ENT("TO", 0x3A, TXPWR_LMT_ETSI),     /* Tonga */
	COUNTRY_CHPLAN_ENT("TR", 0x5E, TXPWR_LMT_ETSI),     /* Turkey, Northern Cyprus */
	COUNTRY_CHPLAN_ENT("TT", 0x76, TXPWR_LMT_FCC),     /* Trinidad & Tobago */
	COUNTRY_CHPLAN_ENT("TV", 0x21, TXPWR_LMT_ETSI),     /* Tuvalu */
	COUNTRY_CHPLAN_ENT("TW", 0x76, TXPWR_LMT_FCC),     /* Taiwan */
	COUNTRY_CHPLAN_ENT("TZ", 0x5E, TXPWR_LMT_ETSI),     /* Tanzania */
	COUNTRY_CHPLAN_ENT("UA", 0x5E, TXPWR_LMT_UKRAINE),     /* Ukraine */
	COUNTRY_CHPLAN_ENT("UG", 0x3A, TXPWR_LMT_ETSI),     /* Uganda */
	COUNTRY_CHPLAN_ENT("US", 0x1B, TXPWR_LMT_FCC),     /* United States of America (USA) */
	COUNTRY_CHPLAN_ENT("UY", 0x30, TXPWR_LMT_FCC),     /* Uruguay */
	COUNTRY_CHPLAN_ENT("UZ", 0x3A, TXPWR_LMT_ETSI),     /* Uzbekistan */
	COUNTRY_CHPLAN_ENT("VA", 0x5E, TXPWR_LMT_ETSI),     /* Holy See (Vatican City) */
	COUNTRY_CHPLAN_ENT("VC", 0x76, TXPWR_LMT_FCC),     /* Saint Vincent and the Grenadines */
	COUNTRY_CHPLAN_ENT("VE", 0x30, TXPWR_LMT_FCC),     /* Venezuela */
	COUNTRY_CHPLAN_ENT("VG", 0x76, TXPWR_LMT_FCC),     /* British Virgin Islands (UK) */
	COUNTRY_CHPLAN_ENT("VI", 0x76, TXPWR_LMT_FCC),     /* United States Virgin Islands (USA) */
	COUNTRY_CHPLAN_ENT("VN", 0x5E, TXPWR_LMT_ETSI),     /* Vietnam */
	COUNTRY_CHPLAN_ENT("VU", 0x26, TXPWR_LMT_ETSI),     /* Vanuatu */
	COUNTRY_CHPLAN_ENT("WF", 0x3A, TXPWR_LMT_ETSI),     /* Wallis and Futuna (France) */
	COUNTRY_CHPLAN_ENT("WS", 0x76, TXPWR_LMT_FCC),     /* Samoa */
	COUNTRY_CHPLAN_ENT("XK", 0x5E, TXPWR_LMT_ETSI),     /* Kosovo */
	COUNTRY_CHPLAN_ENT("YE", 0x3A, TXPWR_LMT_ETSI),     /* Yemen */
	COUNTRY_CHPLAN_ENT("YT", 0x5E, TXPWR_LMT_ETSI),     /* Mayotte (France) */
	COUNTRY_CHPLAN_ENT("ZA", 0x5E, TXPWR_LMT_ETSI),     /* South Africa */
	COUNTRY_CHPLAN_ENT("ZM", 0x5E, TXPWR_LMT_ETSI),     /* Zambia */
	COUNTRY_CHPLAN_ENT("ZW", 0x5E, TXPWR_LMT_ETSI),     /* Zimbabwe */
};

static const struct ieee80211_regdomain *_rtl_regdomain_select(
	u16 country_code)
{
	switch (country_code) {
	case COUNTRY_CODE_FCC:
		return &rtl_regdom_no_midband;
	case COUNTRY_CODE_IC:
		return &rtl_regdom_11;
	case COUNTRY_CODE_TELEC_NETGEAR:
		return &rtl_regdom_60_64;
	case COUNTRY_CODE_ETSI:
	case COUNTRY_CODE_SPAIN:
	case COUNTRY_CODE_FRANCE:
	case COUNTRY_CODE_ISRAEL:
		return &rtl_regdom_12_13;
	case COUNTRY_CODE_MKK:
	case COUNTRY_CODE_MKK1:
	case COUNTRY_CODE_TELEC:
	case COUNTRY_CODE_MIC:
		return &rtl_regdom_14_60_64;
	case COUNTRY_CODE_GLOBAL_DOMAIN:
		return &rtl_regdom_14;
	case COUNTRY_CODE_WORLD_WIDE_13:
	case COUNTRY_CODE_WORLD_WIDE_13_5G_ALL:
		return &rtl_regdom_12_13_5g_all;
	default:
		return &rtl_regdom_no_midband;
	}
}

static bool _rtl_is_radar_freq(u16 center_freq)
{
	return center_freq >= 5260 && center_freq <= 5700;
}

static void _rtl_reg_apply_beaconing_flags(struct wiphy *wiphy,
		enum nl80211_reg_initiator initiator)
{
	enum nl80211_band band;
	struct ieee80211_supported_band *sband;
	const struct ieee80211_reg_rule *reg_rule;
	struct ieee80211_channel *ch;
	unsigned int i;

	for (band = 0; band < NUM_NL80211_BANDS; band++) {

		if (!wiphy->bands[band]) {
			continue;
		}

		sband = wiphy->bands[band];

		for (i = 0; i < sband->n_channels; i++) {
			ch = &sband->channels[i];
			if (_rtl_is_radar_freq(ch->center_freq) ||
				(ch->flags & IEEE80211_CHAN_RADAR)) {
				continue;
			}
			if (initiator == NL80211_REGDOM_SET_BY_COUNTRY_IE) {
				reg_rule = freq_reg_info(wiphy,
										 ch->center_freq);
				if (IS_ERR(reg_rule)) {
					continue;
				}
				/*
				 *If 11d had a rule for this channel ensure
				 *we enable adhoc/beaconing if it allows us to
				 *use it. Note that we would have disabled it
				 *by applying our static world regdomain by
				 *default during init, prior to calling our
				 *regulatory_hint().
				 */

				if (!(reg_rule->flags & NL80211_RRF_NO_IBSS)) {
					ch->flags &= ~IEEE80211_CHAN_NO_IBSS;
				}
				if (!(reg_rule->flags &
					  NL80211_RRF_PASSIVE_SCAN))
					ch->flags &=
						~IEEE80211_CHAN_PASSIVE_SCAN;
			} else {
				if (ch->beacon_found)
					ch->flags &= ~(IEEE80211_CHAN_NO_IBSS |
								   IEEE80211_CHAN_PASSIVE_SCAN);
			}
		}
	}
}

/* Allows active scan scan on Ch 12 and 13 */
static void _rtl_reg_apply_active_scan_flags(struct wiphy *wiphy,
		enum nl80211_reg_initiator
		initiator) {
	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *ch;
	const struct ieee80211_reg_rule *reg_rule;

	if (!wiphy->bands[NL80211_BAND_2GHZ])
	{
		return;
	}
	sband = wiphy->bands[NL80211_BAND_2GHZ];

	/*
	 *If no country IE has been received always enable active scan
	 *on these channels. This is only done for specific regulatory SKUs
	 */
	if (initiator != NL80211_REGDOM_SET_BY_COUNTRY_IE)
	{
		ch = &sband->channels[11];	/* CH 12 */
		if (ch->flags & IEEE80211_CHAN_PASSIVE_SCAN) {
			ch->flags &= ~IEEE80211_CHAN_PASSIVE_SCAN;
		}
		ch = &sband->channels[12];	/* CH 13 */
		if (ch->flags & IEEE80211_CHAN_PASSIVE_SCAN) {
			ch->flags &= ~IEEE80211_CHAN_PASSIVE_SCAN;
		}
		return;
	}

	/*
	 *If a country IE has been recieved check its rule for this
	 *channel first before enabling active scan. The passive scan
	 *would have been enforced by the initial processing of our
	 *custom regulatory domain.
	 */

	ch = &sband->channels[11];	/* CH 12 */
	reg_rule = freq_reg_info(wiphy, ch->center_freq);
	if (!IS_ERR(reg_rule))
	{
		if (!(reg_rule->flags & NL80211_RRF_PASSIVE_SCAN))
			if (ch->flags & IEEE80211_CHAN_PASSIVE_SCAN) {
				ch->flags &= ~IEEE80211_CHAN_PASSIVE_SCAN;
			}
	}

	ch = &sband->channels[12];	/* CH 13 */
	reg_rule = freq_reg_info(wiphy, ch->center_freq);
	if (!IS_ERR(reg_rule))
	{
		if (!(reg_rule->flags & NL80211_RRF_PASSIVE_SCAN))
			if (ch->flags & IEEE80211_CHAN_PASSIVE_SCAN) {
				ch->flags &= ~IEEE80211_CHAN_PASSIVE_SCAN;
			}
	}
}

/*
 *Always apply Radar/DFS rules on
 *freq range 5260 MHz - 5700 MHz
 */
static void _rtl_reg_apply_radar_flags(struct wiphy *wiphy)
{
	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *ch;
	unsigned int i;

	if (!wiphy->bands[NL80211_BAND_5GHZ]) {
		return;
	}

	sband = wiphy->bands[NL80211_BAND_5GHZ];

	for (i = 0; i < sband->n_channels; i++) {
		ch = &sband->channels[i];
		if (!_rtl_is_radar_freq(ch->center_freq)) {
			continue;
		}

		/*
		 *We always enable radar detection/DFS on this
		 *frequency range. Additionally we also apply on
		 *this frequency range:
		 *- If STA mode does not yet have DFS supports disable
		 * active scanning
		 *- If adhoc mode does not support DFS yet then disable
		 * adhoc in the frequency.
		 *- If AP mode does not yet support radar detection/DFS
		 *do not allow AP mode
		 */
		if (!(ch->flags & IEEE80211_CHAN_DISABLED))
			ch->flags |= IEEE80211_CHAN_RADAR |
						 IEEE80211_CHAN_NO_IBSS |
						 IEEE80211_CHAN_PASSIVE_SCAN;
	}
}

static void _rtl_reg_apply_world_flags(struct wiphy *wiphy,
									   enum nl80211_reg_initiator initiator)
{
	_rtl_reg_apply_beaconing_flags(wiphy, initiator);
	_rtl_reg_apply_active_scan_flags(wiphy, initiator);
	return;
}

static u8 channel_plan_to_country_code(u8 channelplan)
{
	switch (channelplan) {
	case 0x20:
	case 0x21:
		return COUNTRY_CODE_WORLD_WIDE_13;
	case 0x22:
		return COUNTRY_CODE_IC;
	case 0x25:
		return COUNTRY_CODE_ETSI;
	case 0x32:
		return COUNTRY_CODE_TELEC_NETGEAR;
	case 0x41:
		return COUNTRY_CODE_GLOBAL_DOMAIN;
	case 0x7f:
		return COUNTRY_CODE_WORLD_WIDE_13_5G_ALL;
	default:
		return COUNTRY_CODE_MAX; /*Error*/
	}
}

static void _rtl_reg_set_country_code(u8 *country)
{
	const struct country_chplan *map = NULL;
	u16 map_sz = 0;
	int i = 0;

	map = country_chplan_map;
	map_sz = sizeof(country_chplan_map) / sizeof(struct country_chplan);
	for (i = 0; i < map_sz; i++) {
		if (strncmp(country, map[i].alpha2, 2) == 0) {
			llhw_wifi_set_ch_plan(map[i].chplan, map[i].pwr_lmt);
			break;
		}
	}
}

void rtw_reg_notifier(struct wiphy *wiphy, struct regulatory_request *request)
{
	switch (request->initiator) {
	case NL80211_REGDOM_SET_BY_USER:
		_rtl_reg_set_country_code(request->alpha2);
		break;
	case NL80211_REGDOM_SET_BY_DRIVER:
	case NL80211_REGDOM_SET_BY_CORE:
	case NL80211_REGDOM_SET_BY_COUNTRY_IE:
	default:
		/* todo */
		break;
	}

	return;
}

int rtw_regd_init(void)
{
	const struct ieee80211_regdomain *regd;
	struct wiphy *wiphy = global_idev.pwiphy_global;
	u8 chplan = 0;
	u16 country_code = COUNTRY_CODE_WORLD_WIDE_13_5G_ALL;

	llhw_wifi_get_chplan(&chplan);
	country_code = channel_plan_to_country_code(chplan);
	dev_dbg(global_idev.fullmac_dev, "%s chplan=0x%x, country_code=0x%x\n", __func__, chplan, country_code);

	wiphy->regulatory_flags |= REGULATORY_CUSTOM_REG;
	wiphy->regulatory_flags &= ~REGULATORY_STRICT_REG;
	wiphy->regulatory_flags &= ~REGULATORY_DISABLE_BEACON_HINTS;

	regd = _rtl_regdomain_select(country_code);
	wiphy_apply_custom_regulatory(wiphy, regd);
	_rtl_reg_apply_radar_flags(wiphy);
	_rtl_reg_apply_world_flags(wiphy, NL80211_REGDOM_SET_BY_DRIVER);

	/* add reg_notifier to set channel plan by user.
	because _rtl_reg_set_country_code must be called after wifi on, rtw_regd_init assignement should be moved to rtw_regd_init */
	wiphy->reg_notifier = rtw_reg_notifier;

	return 0;
}

