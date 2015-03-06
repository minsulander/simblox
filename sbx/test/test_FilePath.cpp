#include <UnitTest++/UnitTest++.h>
#include <sbx/FilePath.h>
#include <iostream>
#include <fstream>
#ifndef WIN32
#include <stdlib.h>
#endif

TEST(FilePath) {
	UNITTEST_TIME_CONSTRAINT(500);

	sbx::FilePath path;

	path.setSeparator(":");
	path.add("/bin::/etc:/usr/bin:");
	CHECK_EQUAL(3, path.size());
#ifndef WIN32
	setenv("TEST_FILEPATH","/usr/local/bin:/usr/global/bin",1);
	path.addEnvironmentVariable("TEST_FILEPATH");
	CHECK_EQUAL(5,path.size());
	{
		std::ifstream ifs(path.find("passwd").c_str());
		CHECK(ifs.good());
	}
#endif
	{
		std::ifstream ifs(path.find("Non-Existant-filE").c_str());
		CHECK(!ifs.good());
	}
	
	CHECK_EQUAL("/a/b", sbx::FilePath::dirname("/a/b/c"));
	CHECK_EQUAL(".", sbx::FilePath::dirname("nodir"));
	CHECK_EQUAL("/", sbx::FilePath::dirname("/onedir"));
	CHECK_EQUAL("/a/b", sbx::FilePath::clean("/a/c/../b"));
	CHECK_EQUAL("/a/b", sbx::FilePath::clean("/a/c/d//../../b"));
	CHECK_EQUAL("../a/b", sbx::FilePath::clean("../a/c/../b"));
	CHECK_EQUAL("../../../../data", sbx::FilePath::clean("../../../../data"));
}

TEST(FilePath_GetFiles) {
	sbx::FilePath path;
#ifdef WIN32
	sbx::FileList files = sbx::FilePath::getDirectoryContents("c:\\");
	path.add("c:\\windows");
	sbx::FileList extfiles = path.getFilesByExtension(".exe");
#else
	sbx::FileList files = sbx::FilePath::getDirectoryContents("/");
	path.add("/etc");
	sbx::FileList extfiles = path.getFilesByExtension("wd");
#endif
	CHECK(files.size() > 0);
	CHECK(extfiles.size() > 0);
}
