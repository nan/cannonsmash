/* $Id$ */

// Copyright (C) 2001-2003  神南 吉宏(Kanna Yoshihiro)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef _LobbyClient_
#define _LobbyClient_

#include <gtk/gtk.h>

#define LOBBYSERVER_NAME	"nan.p.utmc.or.jp"
#define LOBBYSERVER_PORT	(5735)

class PlayerInfo;
class LobbyClientView;

static struct {
  const char *code;
  const char *langname;
  const int langID;
} table[] = {
  { "aa", "Afar", 0x00 },
  { "ab", "Abkhazian", 0x00 },
  { "ae", "Avestan", 0x00 },
  { "af", "Afrikaans", 0x36 },
  { "am", "Amharic", 0x00 },
  { "ar", "Arabic", 0x01 },
  { "as", "Assamese", 0x4d },
  { "ay", "Aymara", 0x00 },
  { "az", "Azerbaijani", 0x2c },
  { "ba", "Bashkir", 0x00 },
  { "be", "Belarusian", 0x23 },
  { "bg", "Bulgarian", 0x02 },
  { "bh", "Bihari", 0x00 },
  { "bi", "Bislama", 0x00 },
  { "bn", "Bengali", 0x45 },
  { "bo", "Tibetan", 0x00 },
  { "br", "Breton", 0x00 },
  { "bs", "Bosnian", 0x00 },
  { "ca", "Catalan", 0x03 },
  { "ce", "Chechen", 0x00 },
  { "ch", "Chamorro", 0x00 },
  { "co", "Corsican", 0x00 },
  { "cs", "Czech", 0x05 },
  { "cu", "Church Slavic", 0x00 },
  { "cv", "Chuvash", 0x00 },
  { "cy", "Welsh", 0x00 },
  { "da", "Danish", 0x06 },
  { "de", "German", 0x07 },
  { "dz", "Dzongkha", 0x00 },
  { "el", "Greek", 0x08 },
  { "en", "English", 0x09 },
  { "eo", "Esperanto", 0x00 },
  { "es", "Spanish", 0x0a },
  { "et", "Estonian", 0x25 },
  { "eu", "Basque", 0x2d },
  { "fa", "Persian", 0x29 },
  { "fi", "Finnish", 0x0b },
  { "fj", "Fijian", 0x00 },
  { "fo", "Faroese", 0x38 },
  { "fr", "French", 0x0c },
  { "fy", "Frisian", 0x00 },
  { "ga", "Irish", 0x00 },
  { "gd", "Scots", 0x00 },
  { "gl", "Galician", 0x56 },
  { "gn", "Guarani", 0x00 },
  { "gu", "Gujarati", 0x47 },
  { "gv", "Manx", 0x00 },
  { "ha", "Hausa", 0x00 },
  { "he", "Hebrew", 0x0d },
  { "hi", "Hindi", 0x39 },
  { "ho", "Hiri Motu", 0x00 },
  { "hr", "Croatian", 0x1a },
  { "hu", "Hungarian", 0x0e },
  { "hy", "Armenian", 0x2b },
  { "hz", "Herero", 0x00 },
  { "ia", "Interlingua", 0x00 },
  { "id", "Indonesian", 0x21 },
  { "ie", "Interlingue", 0x00 },
  { "ik", "Inupiak", 0x00 },
  { "is", "Icelandic", 0x0f },
  { "it", "Italian", 0x10 },
  { "iu", "Inuktitut", 0x00 },
  { "ja", "Japanese", 0x11 },
  { "jw", "Javanese", 0x00 },
  { "ka", "Georgian", 0x37 },
  { "ki", "Kikuyu", 0x00 },
  { "kj", "Kuanyama", 0x00 },
  { "kk", "Kazakh", 0x3f },
  { "kl", "Kalaallisut", 0x00 },
  { "km", "Khmer", 0x00 },
  { "kn", "Kannada", 0x4b },
  { "ko", "Korean", 0x12 },
  { "ks", "Kashmiri", 0x60 },
  { "ku", "Kurdish", 0x00 },
  { "kv", "Komi", 0x00 },
  { "kw", "Cornish", 0x00 },
  { "ky", "Kirghiz", 0x40 },
  { "kok", "Konkani", 0x57 },
  { "la", "Latin", 0x00 },
  { "lb", "Letzeburgesch", 0x00 },
  { "ln", "Lingala", 0x00 },
  { "lo", "Laotian", 0x00 },
  { "lt", "Lithuanian", 0x27 },
  { "lv", "Latvian", 0x26 },
  { "mg", "Malagasy", 0x00 },
  { "mh", "Marshall", 0x00 },
  { "mi", "Maori", 0x00 },
  { "mk", "Macedonian", 0x2f },
  { "ml", "Malayalam", 0x4c },
  { "mn", "Mongolian", 0x50 },
  { "mo", "Moldavian", 0x00 },
  { "mr", "Marathi", 0x4e },
  { "ms", "Malay", 0x3e },
  { "mt", "Maltese", 0x00 },
  { "my", "Burmese", 0x00 },
  { "mni", "Manipuri", 0x58 },
  { "na", "Nauru", 0x00 },
  { "nb", "Norwegian Bokmal", 0x00 },
  { "nd", "North Ndebele", 0x00 },
  { "ne", "Nepali", 0x61 },
  { "ng", "Ndonga", 0x00 },
  { "nl", "Dutch", 0x13 },
  { "nn", "Norwegian Nynorsk", 0x00 },
  { "no", "Norwegian", 0x14 },
  { "nr", "South Ndebele", 0x00 },
  { "nv", "Navajo", 0x00 },
  { "ny", "Nyanja", 0x00 },
  { "oc", "Occitan", 0x00 },
  { "om", "(Afan) Oromo", 0x00 },
  { "or", "Oriya", 0x48 },
  { "os", "Ossetian", 0x00 },
  { "pa", "Punjabi", 0x46 },
  { "pi", "Pali", 0x00 },
  { "pl", "Polish", 0x15 },
  { "ps", "Pashto", 0x00 },
  { "pt", "Portuguese", 0x16 },
  { "qu", "Quechua", 0x00 },
  { "rm", "Rhaeto-Roman", 0x00 },
  { "rn", "Kirundi", 0x00 },
  { "ro", "Romanian", 0x18 },
  { "ru", "Russian", 0x19 },
  { "rw", "Kinyarwanda", 0x00 },
  { "sa", "Sanskrit", 0x4f },
  { "sc", "Sardinian", 0x00 },
  { "sd", "Sindhi", 0x59 },
  { "se", "Northern Sami", 0x00 },
  { "sg", "Sango", 0x00 },
  { "si", "Sinhalese", 0x00 },
  { "sk", "Slovak", 0x1b },
  { "sl", "Slovenian", 0x24 },
  { "sm", "Samoan", 0x00 },
  { "sn", "Shona", 0x00 },
  { "so", "Somali", 0x00 },
  { "sq", "Albanian", 0x1c },
  { "sr", "Serbian", 0x1a },
  { "ss", "Siswati", 0x00 },
  { "st", "Sesotho", 0x00 },
  { "su", "Sundanese", 0x00 },
  { "sv", "Swedish", 0x1d },
  { "sw", "Swahili", 0x41 },
  { "ta", "Tamil", 0x49 },
  { "te", "Telugu", 0x0a },
  { "tg", "Tajik", 0x00 },
  { "th", "Thai", 0x1e },
  { "ti", "Tigrinya", 0x00 },
  { "tk", "Turkmen", 0x00 },
  { "tl", "Tagalog", 0x00 },
  { "tn", "Setswana", 0x00 },
  { "to", "Tonga", 0x00 },
  { "tr", "Turkish", 0x1f },
  { "ts", "Tsonga", 0x00 },
  { "tt", "Tatar", 0x44 },
  { "tw", "Twi", 0x00 },
  { "ty", "Tahitian", 0x00 },
  { "ug", "Uighur", 0x00 },
  { "uk", "Ukrainian", 0x22 },
  { "ur", "Urdu", 0x20 },
  { "uz", "Uzbek", 0x43 },
  { "vi", "Vietnamese", 0x2a },
  { "vo", "Volapuk", 0x00 },
  { "wo", "Wolof", 0x00 },
  { "wen", "Sorbian", 0x00 },
  { "xh", "Xhosa", 0x00 },
  { "yi", "Yiddish", 0x00 },
  { "yo", "Yoruba", 0x00 },
  { "za", "Zhuang", 0x00 },
  { "zh", "Chinese", 0x04 },
  { "zu", "Zulu", 0x00 }, 
  { "", "", -1 }
};

class LobbyClient {
public:
  ~LobbyClient();

  // A sort of singleton
  static LobbyClient* Create();
  static LobbyClient* TheLobbyClient() { return m_lobbyClient; };

  bool Init( char *nickname, char *message );

  int GetSocket() { return m_socket; };
  PlayerInfo *GetPlayerInfo() { return m_player; };
  long GetPlayerNum() { return m_playerNum; };

  static gint PollServerMessage( gpointer data );
  static void Connect( GtkWidget *widget, gpointer data );

  void SendAP( long uniqID );
  void SendSP();
  void SendQP();
  void SendDP( long uniqID);
  void SendQT();
  void SendSC( int score1, int score2 );
  void SendMS( char *message, long channel );

  long m_playerNum;
  PlayerInfo *m_player;

  long m_selected;		// Selected row of the table

  char m_nickname[32];

  long GetLang() { return m_lang; };
protected:
  void ReadUI();
  void UpdateTable();

  void ReadPI();
  void ReadOI();
  void ReadOV();
  void ReadMS();

  LobbyClientView *m_view;

  int m_socket;
  bool m_canBeServer;

  long m_lang;

private:
  LobbyClient();
  static LobbyClient *m_lobbyClient;
};

class PlayerInfo {
public:
  PlayerInfo();
  ~PlayerInfo();

  bool m_canBeServer;
  long m_ID;
  char m_nickname[32];
  char m_message[64];
};

#endif // _LobbyClient_
