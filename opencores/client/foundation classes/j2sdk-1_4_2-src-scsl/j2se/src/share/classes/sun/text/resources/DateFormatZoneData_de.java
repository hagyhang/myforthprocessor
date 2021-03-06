/*
 * @(#)DateFormatZoneData_de.java	1.34 03/05/09
 */

/*
 * Portions Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * (C) Copyright Taligent, Inc. 1996, 1997 - All Rights Reserved
 * (C) Copyright IBM Corp. 1996 - 1998 - All Rights Reserved
 *
 * The original version of this source code and documentation
 * is copyrighted and owned by Taligent, Inc., a wholly-owned
 * subsidiary of IBM. These materials are provided under terms
 * of a License Agreement between Taligent and Sun. This technology
 * is protected by multiple US and International patents.
 *
 * This notice and attribution to Taligent may not be removed.
 * Taligent is a registered trademark of Taligent, Inc.
 *
 */

package sun.text.resources;

/**
 * Supplement package private date-time formatting zone data for DateFormat.
 * DateFormatData used in DateFormat will be initialized by loading the data
 * from LocaleElements and DateFormatZoneData resources. The zone data are
 * represented with the following form:
 * {ID, new String[] {ID, long zone string, short zone string, long daylight
 * string, short daylight string, representative city of zone}}, where ID is
 * NOT localized, but is used to look up the localized timezone data
 * internally. Localizers can localize any zone strings except
 * for the ID of the timezone.
 * Also, localizer should not touch "localPatternChars" entry.
 *
 * @see          Format
 * @see          DateFormatData
 * @see          LocaleElements
 * @see          SimpleDateFormat
 * @see          TimeZone
 * @version      1.23 08/05/01
 * @author       Chen-Lieh Huang
 */
//  German DateFormatZoneData
//
public final class DateFormatZoneData_de extends DateFormatZoneData
{
    /**
     * Overrides DateFormatZoneData
     */
    public Object[][] getContents() {
	String ACT[] = new String[] {"Acre Normalzeit", "ACT",
				     "Acre Sommerzeit", "ACST"};
	String AGT[] = new String[] {"Argentinische Zeit", "ART",
				     "Argentinische Sommerzeit", "ARST"};
	String AKST[] = new String[] {"Alaska Normalzeit", "AKST",
				      "Alaska Sommerzeit", "AKDT"};
	String AMT[] = new String[] {"Amazonas Normalzeit", "AMT",
				     "Amazonas Sommerzeit", "AMST"};
	String ARAST[] = new String[] {"Arabische Normalzeit", "AST",
				       "Arabische Sommerzeit", "ADT"};
	String ARMT[] = new String[] {"Armenische Zeit", "AMT",
				      "Armenische Sommerzeit", "AMST"};
	String AST[] = new String[] {"Atlantik Normalzeit", "AST",
				     "Atlantik Sommerzeit", "ADT"};
	String BDT[] = new String[] {"Bangladesch Zeit", "BDT",
				     "Bangladesch Sommerzeit", "BDST"};
	String BRT[] = new String[] {"Brasilianische Zeit", "BRT",
				     "Brasilianische Sommerzeit", "BRST"};
	String BTT[] = new String[] {"Bhutanische Zeit", "BTT",
				     "Bhutanische Sommerzeit", "BTST"};
	String CAT[] = new String[] {"Zentralafrikanische Zeit", "CAT",
				     "Zentralafrikanische Sommerzeit", "CAST"};
	String CET[] = new String[] {"Zentraleurop\u00e4ische Zeit", "CET",
				     "Zentraleurop\u00e4ische Sommerzeit", "CEST"};
	String CIT[] = new String[] {"Zentralindonesische Zeit", "CIT",
				     "Zentralindonesische Sommerzeit", "CIST"};
	String CLT[] = new String[] {"Chilenische Zeit", "CLT",
				     "Chilenische Sommerzeit", "CLST"};
	String CST[] = new String[] {"Zentrale Normalzeit", "CST",
				     "Zentrale Sommerzeit", "CDT"};
	String CTT[] = new String[] {"Chinesische Normalzeit", "CST",
				     "Chinesische Sommerzeit", "CDT"};
	String EAT[] = new String[] {"Ostafrikanische Zeit", "EAT",
				     "Ostafrikanische Sommerzeit", "EAST"};
	String EET[] = new String[] {"Osteurop\u00e4ische Zeit", "EET",
				     "Osteurop\u00e4ische Sommerzeit", "EEST"};
	String EGT[] = new String[] {"Ostgr\u00f6nl\u00e4ndische Zeit", "EGT",
				     "Ostgr\u00f6nl\u00e4ndische Sommerzeit", "EGST"};
	String EST[] = new String[] {"\u00d6stliche Normalzeit", "EST",
				     "\u00d6stliche Sommerzeit", "EDT"};
	String EST_NSW[] = new String[] {"\u00d6stliche Normalzeit (New South Wales)", "EST",
					 "\u00d6stliche Sommerzeit (New South Wales)", "EST"};
	String GMT[] = new String[] {"Greenwich Mean Time", "GMT",
				   "Greenwich Mean Time", "GMT"};
	String GST[] = new String[] {"Golf Normalzeit", "GST",
				     "Golf Sommerzeit", "GDT"};
	String HAST[] = new String[] {"Hawaii-Aleutische Normalzeit", "HAST",
				      "Hawaii-Aleutische Sommerzeit", "HADT"};
	String HST[] = new String[] {"Hawaii Normalzeit", "HST",
				     "Hawaii Sommerzeit", "HDT"};
	String ICT[] = new String[] {"Indochina Zeit", "ICT",
				     "Indochina Sommerzeit", "ICST"};
	String IRT[] = new String[] {"Iranische Normalzeit", "IRST",
                                     "Iranische Sommerzeit", "IRDT"};
	String IST[] = new String[] {"Indische Normalzeit", "IST",
				     "Indische Sommerzeit", "IDT"};
	String JST[] = new String[] {"Japanische Normalzeit", "JST",
				     "Japanische Sommerzeit", "JDT"};
	String KST[] = new String[] {"Koreanische Normalzeit", "KST",
				     "Koreanische Sommerzeit", "KDT"};
	String MST[] = new String[] {"Rocky Mountains Normalzeit", "MST",
				     "Rocky Mountains Sommerzeit", "MDT"};
	String NST[] = new String[] {"Neufundland Normalzeit", "NST",
				     "Neufundland Sommerzeit", "NDT"};
	String NZST[] = new String[] {"Neuseeland Normalzeit", "NZST",
				      "Neuseeland Sommerzeit", "NZDT"};
	String PKT[] = new String[] {"Pakistanische Zeit", "PKT",
				     "Pakistanische Sommerzeit", "PKST"};
	String PST[] = new String[] {"Pazifische Normalzeit", "PST",
				     "Pazifische Sommerzeit", "PDT"};
	String SAST[] = new String[] {"S\u00fcdafrikanische Normalzeit", "SAST",
				      "S\u00fcdafrikanische Sommerzeit", "SAST"};
	String SBT[] = new String[] {"Salomoninseln Zeit", "SBT",
				     "Salomoninseln Sommerzeit", "SBST"};
	String TMT[] = new String[] {"Turkmenische Zeit", "TMT",
				     "Turkmenische Sommerzeit", "TMST"};
	String ULAT[]= new String[] {"Ulaanbaatar Zeit", "ULAT",
				     "Ulaanbaatar Sommerzeit", "ULAST"};
	String WAT[] = new String[] {"Westafrikanische Zeit", "WAT",
				     "Westafrikanische Sommerzeit", "WAST"};
	String WET[] = new String[] {"Westeurop\u00e4ische Zeit", "WET",
				     "Westeurop\u00e4ische Sommerzeit", "WEST"};
	String WST_AUS[] = new String[] {"Westliche Normalzeit (Australien)", "WST",
					 "Westliche Sommerzeit (Australien)", "WST"};
	String WST_SAMOA[] = new String[] {"West Samoa Zeit", "WST",
					   "West Samoa Sommerzeit", "WSST"};
    String ChST[] = new String[] {"Chamorro Standard Time", "ChST",
                      "Chamorro Daylight Time", "ChDT"};

	return new Object[][] {
	    {"PST", PST},
	    {"America/Los_Angeles", PST},
	    {"MST", MST},
	    {"America/Denver", MST},
	    {"PNT", MST},
	    {"America/Phoenix", MST},
	    {"CST", CST},
	    {"America/Chicago", CST},
	    {"EST", EST},
	    {"America/New_York", EST},
	    {"IET", EST},
	    {"America/Indianapolis", EST},
	    {"HST", HST},
	    {"Pacific/Honolulu", HST},
	    {"AST", AKST},
	    {"America/Anchorage", AKST},
	    {"America/Halifax", AST},
	    {"CNT", NST},
	    {"America/St_Johns", NST},
	    {"ECT", CET},
	    {"Europe/Paris", CET},
	    {"GMT", GMT},
	    {"Africa/Casablanca", WET},
	    {"Asia/Jerusalem", new String[] {"Israelische Normalzeit", "IST",
					     "Israelische Sommerzeit", "IDT"}},
	    {"JST", JST},
	    {"Asia/Tokyo", JST},
	    {"Europe/Bucharest", EET},
	    {"CTT", CTT},
	    {"Asia/Shanghai", CTT},
	    /* Don't change the order of the above zones
	     * to keep compatibility with the previous version.
	     */

	    {"ACT", new String[] {"Zentrale Normalzeit (N\u00f6rdliches Territorium Australien)", "CST",
				  "Zentrale Sommerzeit (N\u00f6rdliches Territorium Australien)", "CDT"}},
	    {"AET", EST_NSW},
	    {"AGT", AGT},
	    {"ART", EET},
	    {"Africa/Abidjan", GMT},
	    {"Africa/Accra", GMT},
	    {"Africa/Addis_Ababa", EAT},
	    {"Africa/Algiers", CET},
	    {"Africa/Asmera", EAT},
	    {"Africa/Bangui", WAT},
	    {"Africa/Banjul", GMT},
	    {"Africa/Bissau", GMT},
	    {"Africa/Blantyre", CAT},
	    {"Africa/Bujumbura", CAT},
	    {"Africa/Cairo", EET},
	    {"Africa/Conakry", GMT},
	    {"Africa/Dakar", GMT},
	    {"Africa/Dar_es_Salaam", EAT},
	    {"Africa/Djibouti", EAT},
	    {"Africa/Douala", WAT},
	    {"Africa/Freetown", GMT},
	    {"Africa/Gaborone", CAT},
	    {"Africa/Harare", CAT},
	    {"Africa/Johannesburg", SAST},
	    {"Africa/Kampala", EAT},
	    {"Africa/Khartoum", EAT},
	    {"Africa/Kigali", CAT},
	    {"Africa/Kinshasa", WAT},
	    {"Africa/Lagos", WAT},
	    {"Africa/Libreville", WAT},
	    {"Africa/Lome", GMT},
	    {"Africa/Luanda", WAT},
	    {"Africa/Lubumbashi", CAT},
	    {"Africa/Lusaka", CAT},
	    {"Africa/Malabo", WAT},
	    {"Africa/Maputo", CAT},
	    {"Africa/Maseru", SAST},
	    {"Africa/Mbabane", SAST},
	    {"Africa/Mogadishu", EAT},
	    {"Africa/Monrovia", GMT},
	    {"Africa/Nairobi", EAT},
	    {"Africa/Ndjamena", WAT},
	    {"Africa/Niamey", WAT},
	    {"Africa/Nouakchott", GMT},
	    {"Africa/Ouagadougou", GMT},
	    {"Africa/Porto-Novo", WAT},
	    {"Africa/Sao_Tome", GMT},
	    {"Africa/Timbuktu", GMT},
	    {"Africa/Tripoli", EET},
	    {"Africa/Tunis", CET},
	    {"Africa/Windhoek", WAT},
	    {"America/Adak", HAST},
	    {"America/Anguilla", AST},
	    {"America/Antigua", AST},
	    {"America/Aruba", AST},
	    {"America/Asuncion", new String[] {"Paraguay Zeit", "PYT",
					       "Paraguay Sommerzeit", "PYST"}},
	    {"America/Barbados", AST},
	    {"America/Belize", CST},
	    {"America/Bogota", new String[] {"Kolumbianische Zeit", "COT",
					     "Kolumbianische Sommerzeit", "COST"}},
	    {"America/Buenos_Aires", AGT},
	    {"America/Caracas", new String[] {"Venezuelanische Zeit", "VET",
					      "Venezuelanische Sommerzeit", "VEST"}},
	    {"America/Cayenne", new String[] {"Franz\u00f6sisch-Guiana Zeit", "GFT",
					      "Franz\u00f6sisch-Guiana Sommerzeit", "GFST"}},
	    {"America/Cayman", EST},
	    {"America/Costa_Rica", CST},
	    {"America/Cuiaba", AMT},
	    {"America/Curacao", AST},
	    {"America/Dawson_Creek", MST},
	    {"America/Dominica", AST},
	    {"America/Edmonton", MST},
	    {"America/El_Salvador", CST},
	    {"America/Fortaleza", BRT},
	    {"America/Godthab", new String[] {"Westgr\u00f6nl\u00e4ndische Zeit", "WGT",
					      "Westgr\u00f6nl\u00e4ndische Sommerzeit", "WGST"}},
	    {"America/Grand_Turk", EST},
	    {"America/Grenada", AST},
	    {"America/Guadeloupe", AST},
	    {"America/Guatemala", CST},
	    {"America/Guayaquil", new String[] {"Ecuadorianische Zeit", "ECT",
						"Ecuadorianische Sommerzeit", "ECST"}},
	    {"America/Guyana", new String[] {"Guyanische Zeit", "GYT",
					     "Guyanische Sommerzeit", "GYST"}},
	    {"America/Havana", CST},
	    {"America/Jamaica", EST},
	    {"America/La_Paz", new String[] {"Bolivianische Zeit", "BOT",
					     "Bolivianische Sommerzeit", "BOST"}},
	    {"America/Lima", new String[] {"Peruanische Zeit", "PET",
					   "Peruanische Sommerzeit", "PEST"}},
	    {"America/Managua", CST},
	    {"America/Manaus", AMT},
	    {"America/Martinique", AST},
	    {"America/Mazatlan", MST},
	    {"America/Mexico_City", CST},
	    {"America/Miquelon", new String[] {"Pierre & Miquelon Normalzeit", "PMST",
					       "Pierre & Miquelon Sommerzeit", "PMDT"}},
	    {"America/Montevideo", new String[] {"Uruguayische Zeit", "UYT",
						 "Uruguayische Sommerzeit", "UYST"}},
	    {"America/Montreal", EST},
	    {"America/Montserrat", AST},
	    {"America/Nassau", EST},
	    {"America/Noronha", new String[] {"Fernando de Noronha Zeit", "FNT",
					      "Fernando de Noronha Sommerzeit", "FNST"}},
	    {"America/Panama", EST},
	    {"America/Paramaribo", new String[] {"Suriname Zeit", "SRT",
						 "Suriname Sommerzeit", "SRST"}},
	    {"America/Port-au-Prince", EST},
	    {"America/Port_of_Spain", AST},
	    {"America/Porto_Acre", ACT},
	    {"America/Puerto_Rico", AST},
	    {"America/Regina", CST},
	    {"America/Rio_Branco", ACT},
	    {"America/Santiago", CLT},
	    {"America/Santo_Domingo", AST},
	    {"America/Sao_Paulo", BRT},
	    {"America/Scoresbysund", EGT},
	    {"America/St_Kitts", AST},
	    {"America/St_Lucia", AST},
	    {"America/St_Thomas", AST},
	    {"America/St_Vincent", AST},
	    {"America/Tegucigalpa", CST},
	    {"America/Thule", AST},
	    {"America/Tijuana", PST},
	    {"America/Tortola", AST},
	    {"America/Vancouver", PST},
	    {"America/Winnipeg", CST},
	    {"Antarctica/Casey", WST_AUS},
	    {"Antarctica/DumontDUrville", new String[] {"Dumont-d'Urville Zeit", "DDUT",
							"Dumont-d'Urville Sommerzeit", "DDUST"}},
	    {"Antarctica/Mawson", new String[] {"Mawson Zeit", "MAWT",
						"Mawson Sommerzeit", "MAWST"}},
	    {"Antarctica/McMurdo", NZST},
	    {"Antarctica/Palmer", CLT},
	    {"Antarctica/Rothera", new String[] {"Rothera Normalzeit", "ROTT",
						 "Rothera Sommerzeit", "ROTST"}},
	    {"Asia/Aden", ARAST},
	    {"Asia/Almaty", new String[] {"Alma Ata Zeit", "ALMT",
					  "Alma-Ata Sommerzeit", "ALMST"}},
	    {"Asia/Amman", EET},
	    {"Asia/Anadyr", new String[] {"Anadyr Zeit", "ANAT",
					  "Anadyr Sommerzeit", "ANAST"}},
	    {"Asia/Aqtau", new String[] {"Aqtau Zeit", "AQTT",
					 "Aqtau Sommerzeit", "AQTST"}},
	    {"Asia/Aqtobe", new String[] {"Aqtobe Zeit", "AQTT",
					  "Aqtobe Sommerzeit", "AQTST"}},
	    {"Asia/Ashgabat", TMT},
	    {"Asia/Ashkhabad", TMT},
	    {"Asia/Baghdad", ARAST},
	    {"Asia/Bahrain", ARAST},
	    {"Asia/Baku", new String[] {"Aserbaidschanische Zeit", "AZT",
					"Aserbaidschanische Sommerzeit", "AZST"}},
	    {"Asia/Bangkok", ICT},
	    {"Asia/Beirut", EET},
	    {"Asia/Bishkek", new String[] {"Kirgisische Zeit", "KGT",
					   "Kirgisische Sommerzeit", "KGST"}},
	    {"Asia/Brunei", new String[] {"Brunei Zeit", "BNT",
					  "Brunei Sommerzeit", "BNST"}},
	    {"Asia/Calcutta", IST},
	    {"Asia/Colombo", new String[] {"Sri Lanka Zeit", "LKT",
					   "Sri Lanka Sommerzeit", "LKST"}},
	    {"Asia/Dacca", BDT},
	    {"Asia/Dhaka", BDT},
	    {"Asia/Damascus", EET},
	    {"Asia/Dubai", GST},
	    {"Asia/Dushanbe", new String[] {"Tadschikische Zeit", "TJT",
					    "Tadschikische Sommerzeit", "TJST"}},
	    {"Asia/Hong_Kong", new String[] {"Hongkong Zeit", "HKT",
					     "Hongkong Sommerzeit", "HKST"}},
	    {"Asia/Irkutsk", new String[] {"Irkutsk Zeit", "IRKT",
					   "Irkutsk Sommerzeit", "IRKST"}},
        {"Asia/Jakarta", new String[] {"West Indonesia Time", "WIT",
                       "West Indonesia Summer Time", "WIST"}},
        {"Asia/Jayapura", new String[] {"East Indonesia Time", "EIT",
                        "East Indonesia Summer Time", "EIST"}},
	    {"Asia/Kabul", new String[] {"Afghanistanische Zeit", "AFT",
					 "Afghanistanische Sommerzeit", "AFST"}},
	    {"Asia/Kamchatka", new String[] {"Petropawlowsk-Kamtschatkische Zeit", "PETT",
					     "Petropawlowsk-Kamtschatkische Sommerzeit", "PETST"}},
	    {"Asia/Karachi", PKT},
	    {"Asia/Katmandu", new String[] {"Nepalesische Zeit", "NPT",
					    "Nepalesische Sommerzeit", "NPST"}},
	    {"Asia/Krasnoyarsk", new String[] {"Krasnojarsker Zeit", "KRAT",
					       "Krasnojarsker Sommerzeit", "KRAST"}},
	    {"Asia/Kuala_Lumpur", new String[] {"Malayische Zeit", "MYT",
						"Malayische Sommerzeit", "MYST"}},
	    {"Asia/Kuwait", ARAST},
	    {"Asia/Macao", CTT},
	    {"Asia/Macau", CTT},
	    {"Asia/Magadan", new String[] {"Magadanische Zeit", "MAGT",
					   "Magadanische Sommerzeit", "MAGST"}},
	    {"Asia/Makassar", CIT},
	    {"Asia/Manila", new String[] {"Philippinische Zeit", "PHT",
					  "Philippinische Sommerzeit", "PHST"}},
	    {"Asia/Muscat", GST},
	    {"Asia/Nicosia", EET},
	    {"Asia/Novosibirsk", new String[] {"Nowosibirsker Zeit", "NOVT",
					       "Nowosibirsker Sommerzeit", "NOVST"}},
	    {"Asia/Oral", new String[] {"Oral Zeit", "ORAT",
					"Oral Sommerzeit", "ORAST"}},
	    {"Asia/Phnom_Penh", ICT},
	    {"Asia/Pyongyang", KST},
	    {"Asia/Qatar", ARAST},
	    {"Asia/Qyzylorda", new String[] {"Qyzylorda Zeit", "QYZT",
					     "Qyzylorda Sommerzeit", "QYZST"}},
	    {"Asia/Rangoon", new String[] {"Myanmar Zeit", "MMT",
					   "Myanmar Sommerzeit", "MMST"}},
	    {"Asia/Riyadh", ARAST},
	    {"Asia/Saigon", ICT},
	    {"Asia/Seoul", KST},
	    {"Asia/Singapore", new String[] {"Singapur Zeit", "SGT",
					     "Singapur Sommerzeit", "SGST"}},
	    {"Asia/Taipei", CTT},
	    {"Asia/Tashkent", new String[] {"Usbekistanische Zeit", "UZT",
					    "Usbekistanische Sommerzeit", "UZST"}},
	    {"Asia/Tbilisi", new String[] {"Georgische Zeit", "GET",
					   "Georgische Sommerzeit", "GEST"}},
	    {"Asia/Tehran", IRT},
	    {"Asia/Thimbu", BTT},
	    {"Asia/Thimphu", BTT},
	    {"Asia/Ujung_Pandang", CIT},
	    {"Asia/Ulaanbaatar", ULAT},
	    {"Asia/Ulan_Bator", ULAT},
	    {"Asia/Vientiane", ICT},
	    {"Asia/Vladivostok", new String[] {"Wladiwostok Zeit", "VLAT",
					       "Wladiwostok Sommerzeit", "VLAST"}},
	    {"Asia/Yakutsk", new String[] {"Jakutsk Zeit", "YAKT",
					   "Jakutsk Sommerzeit", "YAKST"}},
	    {"Asia/Yekaterinburg", new String[] {"Jekaterinburger Zeit", "YEKT",
						 "Jekaterinburger Sommerzeit", "YEKST"}},
	    {"Asia/Yerevan", ARMT},
	    {"Atlantic/Azores", new String[] {"Azoren Zeit", "AZOT",
					      "Azoren Sommerzeit", "AZOST"}},
	    {"Atlantic/Bermuda", AST},
	    {"Atlantic/Canary", WET},
	    {"Atlantic/Cape_Verde", new String[] {"Kap Verde Zeit", "CVT",
						  "Kap Verde Sommerzeit", "CVST"}},
	    {"Atlantic/Faeroe", WET},
	    {"Atlantic/Jan_Mayen", EGT},
	    {"Atlantic/Reykjavik", GMT},
	    {"Atlantic/South_Georgia", new String[] {"South Georgia Normalzeit", "GST",
						     "South Georgia Sommerzeit", "GDT"}},
	    {"Atlantic/St_Helena", GMT},
	    {"Atlantic/Stanley", new String[] {"Falkland Inseln Zeit", "FKT",
					       "Falkland Inseln Sommerzeit", "FKST"}},
	    {"Australia/Adelaide", new String[] {"Zentrale Normalzeit (S\u00fcdaustralien)", "CST",
						 "Zentrale Sommerzeit (S\u00fcdaustralien)", "CST"}},
	    {"Australia/Brisbane", new String[] {"\u00d6stliche Normalzeit (Queensland)", "EST",
						 "\u00d6stliche Sommerzeit (Queensland)", "EST"}},
	    {"Australia/Broken_Hill", new String[] {"Zentrale Normalzeit (S\u00fcdaustralien/New South Wales)", "CST",
						    "Zentrale Sommerzeit (S\u00fcdaustralien/New South Wales)", "CST"}},
	    {"Australia/Darwin", new String[] {"Zentrale Normalzeit (N\u00f6rdliches Territorium Australien)", "CST",
					       "Zentrale Sommerzeit (N\u00f6rdliches Territorium Australien)", "CST"}},
	    {"Australia/Hobart", new String[] {"\u00d6stliche Normalzeit (Tasmanien)", "EST",
					       "\u00d6stliche Sommerzeit (Tasmanien)", "EST"}},
	    {"Australia/Lord_Howe", new String[] {"Lord Howe Normalzeit", "LHST",
						  "Lord Howe Sommerzeit", "LHST"}},
	    {"Australia/Perth", WST_AUS},
	    {"Australia/Sydney", EST_NSW},
	    {"BET", BRT},
	    {"BST", BDT},
	    {"CAT", CAT},
	    {"EAT", EAT},
	    {"EET", EET},
	    {"Europe/Amsterdam", CET},
	    {"Europe/Andorra", CET},
	    {"Europe/Athens", EET},
	    {"Europe/Belgrade", CET},
	    {"Europe/Berlin", CET},
	    {"Europe/Brussels", CET},
	    {"Europe/Budapest", CET},
	    {"Europe/Chisinau", EET},
	    {"Europe/Copenhagen", CET},
	    {"Europe/Dublin", new String[] {"Greenwich Mean Time", "GMT",
					    "Irische Sommerzeit", "IST"}},
	    {"Europe/Gibraltar", CET},
	    {"Europe/Helsinki", EET},
	    {"Europe/Istanbul", EET},
	    {"Europe/Kaliningrad", EET},
	    {"Europe/Kiev", EET},
	    {"Europe/Lisbon", WET},
	    {"Europe/London", new String[] {"Greenwich Mean Time", "GMT",
					    "Britische Sommerzeit", "BST"}},
	    {"Europe/Luxembourg", CET},
	    {"Europe/Madrid", CET},
	    {"Europe/Malta", CET},
	    {"Europe/Minsk", EET},
	    {"Europe/Monaco", CET},
	    {"Europe/Moscow", new String[] {"Moskauer Normalzeit", "MSK",
					    "Moskauer Sommerzeit", "MSD"}},
	    {"Europe/Oslo", CET},
	    {"Europe/Prague", CET},
	    {"Europe/Riga", EET},
	    {"Europe/Rome", CET},
	    {"Europe/Samara", new String[] {"Samarische Zeit", "SAMT",
					    "Samarische Sommerzeit", "SAMST"}},
	    {"Europe/Simferopol", EET},
	    {"Europe/Sofia", EET},
	    {"Europe/Stockholm", CET},
	    {"Europe/Tallinn", EET},
	    {"Europe/Tirane", CET},
	    {"Europe/Vaduz", CET},
	    {"Europe/Vienna", CET},
	    {"Europe/Vilnius", EET},
	    {"Europe/Warsaw", CET},
	    {"Europe/Zurich", CET},
	    {"IST", IST},
	    {"Indian/Antananarivo", EAT},
	    {"Indian/Chagos", new String[] {"Indischer Ozean Territorium Zeit", "IOT",
					    "Indischer Ozean Territorium Sommerzeit", "IOST"}},
	    {"Indian/Christmas", new String[] {"Christmas Island Zeit", "CXT",
					       "Christmas Island Sommerzeit", "CXST"}},
	    {"Indian/Cocos", new String[] {"Cocos Islands Zeit", "CCT",
					   "Cocos Islands Sommerzeit", "CCST"}},
	    {"Indian/Comoro", EAT},
	    {"Indian/Kerguelen", new String[] {"Franz\u00f6sisch S\u00fcd- u. Antarktische Landzeit", "TFT",
					       "Franz\u00f6sisch S\u00fcd- u. Antarktische Landsommerzeit", "TFST"}},
	    {"Indian/Mahe", new String[] {"Seychellen Zeit", "SCT",
					  "Seychellen Sommerzeit", "SCST"}},
	    {"Indian/Maldives", new String[] {"Maledivische Zeit", "MVT",
					      "Maledivische Sommerzeit", "MVST"}},
	    {"Indian/Mauritius", new String[] {"Mauritius Zeit", "MUT",
					       "Mauritius Sommerzeit", "MUST"}},
	    {"Indian/Mayotte", EAT},
	    {"Indian/Reunion", new String[] {"Reunion Zeit", "RET",
					     "Reunion Sommerzeit", "REST"}},
	    {"MET", IRT},
	    {"MIT", WST_SAMOA},
	    {"NET", ARMT},
	    {"NST", NZST},
	    {"PLT", PKT},
	    {"PRT", AST},
	    {"Pacific/Apia", WST_SAMOA},
	    {"Pacific/Auckland", NZST},
	    {"Pacific/Chatham", new String[] {"Chatham Normalzeit", "CHAST",
					      "Chatham Sommerzeit", "CHADT"}},
	    {"Pacific/Easter", new String[] {"Osterinseln Zeit", "EAST",
					     "Osterinseln Sommerzeit", "EASST"}},
	    {"Pacific/Efate", new String[] {"Vanuatu Zeit", "VUT",
					    "Vanuatu Sommerzeit", "VUST"}},
	    {"Pacific/Enderbury", new String[] {"Phoenix Inseln Zeit", "PHOT",
						"Phoenix Inseln Sommerzeit", "PHOST"}},
	    {"Pacific/Fakaofo", new String[] {"Tokelau Zeit", "TKT",
					      "Tokelau Sommerzeit", "TKST"}},
	    {"Pacific/Fiji", new String[] {"Fidschi Zeit", "FJT",
					   "Fidschi Sommerzeit", "FJST"}},
	    {"Pacific/Funafuti", new String[] {"Tuvalu Zeit", "TVT",
					       "Tuvalu Sommerzeit", "TVST"}},
	    {"Pacific/Galapagos", new String[] {"Galapagos Zeit", "GALT",
						"Galapagos Sommerzeit", "GALST"}},
	    {"Pacific/Gambier", new String[] {"Gambier Zeit", "GAMT",
					      "Gambier Sommerzeit", "GAMST"}},
	    {"Pacific/Guadalcanal", SBT},
        {"Pacific/Guam", ChST},
	    {"Pacific/Kiritimati", new String[] {"Line Inseln Zeit", "LINT",
						 "Line Inseln Sommerzeit", "LINST"}},
	    {"Pacific/Kosrae", new String[] {"Kosrae Zeit", "KOST",
					     "Kosrae Sommerzeit", "KOSST"}},
	    {"Pacific/Majuro", new String[] {"Marshallinselzeit", "MHT",
					     "Marshallinselsommerzeit", "MHST"}},
	    {"Pacific/Marquesas", new String[] {"Marquesas Zeit", "MART",
						"Marquesas Sommerzeit", "MARST"}},
	    {"Pacific/Nauru", new String[] {"Nauru Zeit", "NRT",
					    "Nauru Sommerzeit", "NRST"}},
	    {"Pacific/Niue", new String[] {"Niue Zeit", "NUT",
					   "Niue Sommerzeit", "NUST"}},
	    {"Pacific/Norfolk", new String[] {"Norfolk Zeit", "NFT",
					      "Norfolk Sommerzeit", "NFST"}},
	    {"Pacific/Noumea", new String[] {"Neukaledonische Zeit", "NCT",
					     "Neukaledonische Sommerzeit", "NCST"}},
	    {"Pacific/Pago_Pago", new String[] {"Samoa Normalzeit", "SST",
						"Samoa Sommerzeit", "SDT"}},
	    {"Pacific/Palau", new String[] {"Palau Zeit", "PWT",
					    "Palau Sommerzeit", "PWST"}},
	    {"Pacific/Pitcairn", new String[] {"Pitcairn Normalzeit", "PST",
					       "Pitcairn Sommerzeit", "PDT"}},
	    {"Pacific/Ponape", new String[] {"Ponape Zeit", "PONT",
					     "Ponape Sommerzeit", "PONST"}},
	    {"Pacific/Port_Moresby", new String[] {"Papua-Neuguinea Zeit", "PGT",
						   "Papua-Neuguinea Sommerzeit", "PGST"}},
	    {"Pacific/Rarotonga", new String[] {"Cook-Inseln Zeit", "CKT",
						"Cook-Inseln Sommerzeit", "CKST"}},
        {"Pacific/Saipan", ChST},
	    {"Pacific/Tahiti", new String[] {"Tahiti Zeit", "TAHT",
					     "Tahiti Sommerzeit", "TAHST"}},
	    {"Pacific/Tarawa", new String[] {"Gilbert-Inseln Zeit", "GILT",
					     "Gilbert-Inseln Sommerzeit", "GILST"}},
	    {"Pacific/Tongatapu", new String[] {"Tonga Zeit", "TOT",
						"Tonga Sommerzeit", "TOST"}},
	    {"Pacific/Truk", new String[] {"Truk Zeit", "TRUT",
					   "Truk Sommerzeit", "TRUST"}},
	    {"Pacific/Wake", new String[] {"Wake Zeit", "WAKT",
					   "Wake Sommerzeit", "WAKST"}},
	    {"Pacific/Wallis", new String[] {"Wallis u. Futuna Zeit", "WFT",
					     "Wallis u. Futuna Sommerzeit", "WFST"}},
	    {"SST", SBT},
	    {"UTC", new String[] {"Coordinated Universal Time", "UTC",
			 	  "Coordinated Universal Time", "UTC"}},
	    {"VST", ICT},
	    {"WET", WET},

            {"localPatternChars", "GyMdkHmsSEDFwWahKzZ"},
        };
    }
}



