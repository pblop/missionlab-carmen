#!/bin/bash
rm mlab.tags
find . -name '*.h' | etags --members -o mlab.tags -
find . -name '*.hpp' | etags --members -a -o mlab.tags -
find . -name '*.cpp' | etags --members -a -o mlab.tags -
find . -name '*.cc' | etags --members -a -o mlab.tags -
find . -name '*.c' | etags --members -a -o mlab.tags -

