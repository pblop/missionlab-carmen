///////////////////////////////////////////////////////////////////////////////
//
//                                 time.h
//
// This file declares classes for dealing with time and timing
//
// Classes defined for export:
//    IPTime
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_time_h
#define ip_time_h

class IPTime {
public:
    IPTime();
    IPTime(double);
    IPTime(int, int);

    static IPTime Current();

    IPTime& operator+=(const IPTime&);
    IPTime& operator-=(const IPTime&);
    int Set() { return _sec != 0 || _msec != 0; }

    double Value() const;
    void Value(int&, int&) const;
    int Sec() const { return _sec; }
    int MSec() const { return _msec; }

private:
    int _sec;
    int _msec;

friend IPTime operator+(const IPTime&, const IPTime&);
friend IPTime operator-(const IPTime&, const IPTime&);
};
    
#endif
