/**
 * @file
 *
 * @brief Tests for blockresolver plugin
 *
 * @copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kdbconfig.h>
#include <kdbinternal.h>

#include <tests_plugin.h>

static void test_BlockresolverRead (char * fileName)
{
	Key * parentKey = keyNew ("user/test/blockresolver-read", KEY_VALUE, srcdir_file (fileName), KEY_END);
	KeySet * conf = ksNew (10, keyNew ("system/path", KEY_VALUE, srcdir_file (fileName), KEY_END),
			       keyNew ("system/identifier", KEY_VALUE, "### block config", KEY_END), KS_END);
	KeySet * modules = ksNew (0, KS_END);
	KeySet * ks = ksNew (0, KS_END);
	elektraModulesInit (modules, 0);
	Plugin * resolver = elektraPluginOpen ("blockresolver", modules, ksDup (conf), 0);
	succeed_if (resolver->kdbGet (resolver, ks, parentKey) >= 0, "blockresolver->kdbGet failed");
	Plugin * storage = elektraPluginOpen ("ini", modules, ksNew (0, KS_END), 0);
	succeed_if (storage->kdbGet (storage, ks, parentKey) >= 0, "storage->kdbGet failed");
	succeed_if (!strcmp (keyString (ksLookupByName (ks, "user/test/blockresolver-read/section/key", 0)), "inside block"),
		    "blockresolver failed to resolve requested block");
	elektraPluginClose (storage, 0);
	elektraPluginClose (resolver, 0);
	ksDel (conf);
	ksDel (ks);
	elektraModulesClose (modules, 0);
	ksDel (modules);
	keyDel (parentKey);
}

static void test_BlockresolverWrite (char * fileName, char * compareName)
{
	FILE * fin = fopen (srcdir_file (fileName), "r");
	char buffer[1024];
	const char * foutname = elektraFilename ();
	FILE * fout = fopen (foutname, "w");
	while (fgets (buffer, sizeof (buffer), fin))
	{
		fputs (buffer, fout);
	}
	fclose (fin);
	fclose (fout);

	Key * parentKey = keyNew ("user/test/blockresolver-write", KEY_VALUE, foutname, KEY_END);
	KeySet * conf = ksNew (10, keyNew ("system/path", KEY_VALUE, foutname, KEY_END),
			       keyNew ("system/identifier", KEY_VALUE, "### block config", KEY_END), KS_END);
	KeySet * modules = ksNew (0, KS_END);
	KeySet * ks = ksNew (0, KS_END);
	elektraModulesInit (modules, 0);
	Plugin * resolver = elektraPluginOpen ("blockresolver", modules, ksDup (conf), 0);
	succeed_if (resolver->kdbGet (resolver, ks, parentKey) >= 0, "blockresolver->kdbGet failed");
	Plugin * storage = elektraPluginOpen ("ini", modules, ksNew (0, KS_END), 0);
	succeed_if (storage->kdbGet (storage, ks, parentKey) >= 0, "storage->kdbGet failed");
	keySetString (ksLookupByName (ks, "user/test/blockresolver-write/section/key", 0), "only the inside has changed");
	succeed_if (storage->kdbSet (storage, ks, parentKey) >= 0, "storage->kdbSet failed");
	succeed_if (resolver->kdbSet (resolver, ks, parentKey) >= 0, "blockresolver->kdbSet failed");
	succeed_if (resolver->kdbSet (resolver, ks, parentKey) >= 0, "blockresolver->kdbSet failed");

	succeed_if (compare_line_files (srcdir_file (compareName), foutname), "files do not match as expected");

	elektraPluginClose (storage, 0);
	elektraPluginClose (resolver, 0);
	ksDel (conf);
	ksDel (ks);
	elektraModulesClose (modules, 0);
	ksDel (modules);
	keyDel (parentKey);
}

int main (int argc, char ** argv)
{
	printf ("BLOCKRESOLVER     TESTS\n");
	printf ("==================\n\n");

	init (argc, argv);

	test_BlockresolverRead ("blockresolver/test.block");
	test_BlockresolverWrite ("blockresolver/test.block", "blockresolver/compare.block");

	printf ("\ntestmod_blockresolver RESULTS: %d test(s) done. %d error(s).\n", nbTest, nbError);

	return nbError;
}
