unsigned char doc_tests_embedding_test_mcfg[] =
    "sector embedding_test\n  section sect1\n    str foo 'bar'\n    u16 "
    "someu16 1337\n\n    str local_embed_test '$(foo)'\n  end\nend\nsector "
    "list_src\n  section sect1\n    list str somelist 'aasd', 'bebis', "
    "'$(/embedding_test/sect1/someu16)'\n  end\nend\nsector test\n  section "
    "sectneg1\n    i16 test -200\n    str a 'üüäü'\n  end\n\n  section sect1\n "
    "   bool booltest true\n    bool b2bool2test false\n\n    list bool "
    "boolllist true, false, false, true,\n    true\n\n    u8 test 200\n    str "
    "stest 'asdasd $(sectneg1/a)'\n    str dest 'foo "
    "$(/embedding_test/sect1/foo) with someu16\n    $(booltest) $(b2bool2test) "
    "$(boolllist)\n    $(/embedding_test/sect1/someu16) this should appear "
    "normal -> \\$(aa) ''\n    abc__<$(/list_src/sect1/somelist)>@@ a\n    "
    "$(/bogus/field) $(test) $(sectneg1/test)\n    $(stest)\n  '\n  end\nend\n";
unsigned int doc_tests_embedding_test_mcfg_len =
    sizeof(doc_tests_embedding_test_mcfg);
