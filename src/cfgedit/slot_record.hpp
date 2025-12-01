/**********************************************************************
 **                                                                  **
 **                         slot_record.hpp                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: slot_record.hpp,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef SLOT_RECORD_HPP
#define SLOT_RECORD_HPP

// Define the glyph class
class slot_record 
{
  public:
      bool            valid;  // Is this record loaded.
      char           *name;
      char          **value;  // Pointer to the value in sym_rec
      char          **index;  // Pointer to the index in sym_rec
      bool            list_header;
      int             rows_in_list;  // if list_header, set to number of entries
				     // in this list (0..) without extra_rec
      bool            in_list;
      bool            extra_slot;
      bool            is_const;
      bool            is_readonly;
      bool            is_cmds_list;
      bool            is_pu;
      Symbol        **input_generator_;
      Symbol         *src_symbol;
      Symbol         *parm_rec;
      Symbol         *data_type;
      Widget          input_widget;
      Widget          index_widget;	// If in_list

      slot_record()
      {
	 memset(this,0,sizeof(*this));
      }

      ~slot_record() { }
};

slot_record *build_slot_info(Symbol *sym,bool detail,int &level,
                             int &num_slots,int &num_cols);
#endif


///////////////////////////////////////////////////////////////////////
// $Log: slot_record.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:34  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.12  2000/02/23 18:23:45  endo
// Checked in for doug.
// Code changed, so that the library agents
// are readonly in the editor. This will
// keep users from making changes to parameters
// (which will be lost) and also make the
// istinction between user space agents and
// ibrary agents explicit.
// Readonly glyphs have a new text line added
// which states that the object is read only
// and lists the library it comes from and
// the description and any constant fields
// are not editable.
//
// Revision 1.11  1997/02/14 15:55:46  zchen
// *** empty log message ***
//
// Revision 1.10  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.9  1996/01/27  00:09:24  doug
// added grouped lists of parameters
//
// Revision 1.8  1996/01/25  14:53:34  doug
// *** empty log message ***
//
// Revision 1.7  1995/11/12  22:39:46  doug
// *** empty log message ***
//
// Revision 1.6  1995/11/07  14:30:36  doug
// *** empty log message ***
//
// Revision 1.5  1995/09/19  15:33:19  doug
// The executive module is now working
//
// Revision 1.4  1995/09/15  22:02:20  doug
// added support for the command list in the executive
//
// Revision 1.3  1995/06/29  18:14:34  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
