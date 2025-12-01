

#include "mic.h"
#include "commMsg.h"

namespace sara
{

/*
{
public:
   /// Define the MsgType handle
   typedef uword MsgType_T;
   static const MsgType_T INVALID_MSG_TYPE = 0;

   /// Helper functions to read and write the MsgType
   bool MsgType_to_stream(stringstream *ost, MsgType_T MsgType) const;
   static MsgType_T MsgType_from_stream(stringstream *ost);

   // ------------------------- start of data ------------------------
   // Holds the type of this message to assist in low cost RTT checking.
   MsgType_T msgType;

   // The name of the sender.
   string senderID;

   // The address of the sender
   ComAddr senderAddr;
   // -------------------------- end of data -------------------------
 
   // Write the msg to a stream buffer in network byte order.
   // Returns true on success, false if the operation fails
   virtual bool to_stream(stringstream *ost) const = 0;

   /// Create a copy of the msg 
   virtual commMsg *clone() = 0;

   /// Copy the information from the source base record into this record
   /// to complete a clone operation
   void cloneBase(commMsg *src);

   // Helper functions to stream in and out basic types.
   static bool out_byte(stringstream *st, const byte val);
   static bool in_byte(stringstream *st, byte &val);
   static bool out_ubyte(stringstream *st, const ubyte val);
   static bool in_ubyte(stringstream *st, ubyte &val);
   static bool out_word(stringstream *st, const word val);
   static bool in_word(stringstream *st, word &val);
   static bool out_uword(stringstream *st, const uword val);
   static bool in_uword(stringstream *st, uword &val);
   static bool out_short(stringstream *st, const short val);
   static bool in_short(stringstream *st, short &val);
   static bool out_int(stringstream *st, const int val);
   static bool in_int(stringstream *st, int &val);
   static bool out_uint(stringstream *st, const uint val);
   static bool in_uint(stringstream *st, uint &val);
   static bool out_long(stringstream *st, const long val);
   static bool in_long(stringstream *st, long &val);
   static bool out_ulong(stringstream *st, const ulong val);
   static bool in_ulong(stringstream *st, ulong &val);
   static bool out_string(stringstream *st, const string &val);
   static bool in_string(stringstream *st, string &val);
   static bool out_float(stringstream *st, const float val);
   static bool in_float(stringstream *st, float &val);
   static bool out_double(stringstream *st, const double val);
   static bool in_double(stringstream *st, double &val);
   static bool out_bool(stringstream *st, const bool val);
   static bool in_bool(stringstream *st, bool &val);

   // Constructor
   commMsg(MsgType_T t = INVALID_MSG_TYPE);
   commMsg(const commMsg &msg);

   // Destructor
   virtual ~commMsg() {};
};
*/

int main(int, char **)
{
   {
      stringstream st;
      commMsg::MsgType_T MsgType = (commMsg::MsgType_T)-1;
      if( !commMsg::MsgType_to_stream(&st, MsgType) )
      {
         cerr << "MsgType_to_stream failed with " << (uint)MsgType << endl;
         exit(1);
      }

      commMsg::MsgType_T rtn = commMsg::MsgType_from_stream(&st);

      if( rtn != MsgType )
      {
         cerr << "MsgType_from_stream returned " << (uint)rtn << " instead of " << (uint)MsgType << endl;
         exit(1);
      }

      if( sizeof(commMsg::MsgType_T) != 2 )
      {
         cerr << "sizeof(MsgType_T) = " << sizeof(commMsg::MsgType_T) << " instead of 2" << endl;
         exit(1);
      }
   }

   {
      stringstream st;
      uint out = (uint)-1;
      if( !commMsg::out_uint(&st, out) )
      {
         cerr << "out_uint failed with " << out << endl;
         exit(1);
      }

      uint rtn;
      if( !commMsg::in_uint(&st, rtn) )
      {
         cerr << "in_uint failed" << endl;
         exit(1);
      }

      if( rtn != out )
      {
         cerr << "in_uint returned " << (uint)rtn << " instead of " << (uint)out << endl;
         exit(1);
      }

      if( sizeof(uint) != 4 )
      {
         cerr << "sizeof(uint) = " << sizeof(uint) << " instead of 4" << endl;
         exit(1);
      }
   }
}
}
