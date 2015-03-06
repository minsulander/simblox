Welcome to libsmrt, a smart pointer library.

This is a fork of OpenSceneGraphs implementation of reference counted smart pointers. for more information please visit http://www.openscenegraph.org

Caveats:

	Generally you can mix osg and smrt ref_ptrs because they use their templateness to simply use any objects ref, unref, functions. However, you cannot create 'smrt::observer_ptr's to 'osg::Referenced' objects because 'osg::Referenced' refers specifically to 'osg::observer_ptr' for removal.



Andrew Somerville
andy.somerville@gmail.com
2007-April-30

