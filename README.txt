$Id: README.txt,v 1.2 2010/02/09 00:13:20 q0rban Exp $

********************************************************************************
Region Manager provides a simple interface to manage blocks, and combined with
the nodeblock module, add new blocks to regions.  You can specify which regions
for each theme can be managed.  After clicking the link to configure a region,
an interface is provided for managing which blocks are visible ON THE PAGE from
which the link was clicked.

The list of which blocks are available can be configured per theme, per region,
per module, or even per block.  For example, you can specify that menu module
blocks are only available in the header, or that the navigation menu block is
not available for any region.

Recommended additional modules are NodeBlock, and Visibility API.

Region Manager also allows you to implement your own block creation methods.
Please see region_manager.nodeblock.inc for an example.  Note: You may need to
implement your own submit handler, depending on what kind of block you are 
creating.  See region_manager_nodeapi() for an example.