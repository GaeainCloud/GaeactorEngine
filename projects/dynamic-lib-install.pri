
# 设定了目标路径，生成的插件文件自动会进入这些路径下，不必再追加INSTALLS

defineTest(copyDynamicLibrary) {

win32: {

CONFIG(debug, debug|release): {

    EXTRA_BINFILES += $$quote($$_PRO_FILE_PWD_/../lib_x64/win/Debug/$$1/debug/*.pdb)
    EXTRA_BINFILES += $$quote($$_PRO_FILE_PWD_/../lib_x64/win/Debug/$$1/debug/*.dll)

    EXTRA_BINFILES_WIN = $${EXTRA_BINFILES}
    EXTRA_BINFILES_WIN ~= s,/,\\,g

    DESTDIR_WIN = $$_PRO_FILE_PWD_/../../bin/win64/debug
    DESTDIR_WIN ~= s,/,\\,g

    QMAKE_POST_LINK += $$quote(if not exist $${DESTDIR_WIN} mkdir $${DESTDIR_WIN} $$escape_expand(\n\t))

    for(FILE,EXTRA_BINFILES_WIN){
        QMAKE_POST_LINK += $$quote(cmd /c copy /y $${FILE} $${DESTDIR_WIN}$$escape_expand(\n\t))
    }

    ###### copy sdk files
    EXTRA_SDKFILES += $$quote($$_PRO_FILE_PWD_/../lib_x64/win/Debug/$$1/debug/*.lib)

    EXTRA_SDKFILES_WIN = $${EXTRA_SDKFILES}
    EXTRA_SDKFILES_WIN ~= s,/,\\,g

    DESTDIR_SDK_WIN = $$_PRO_FILE_PWD_/../../bin/win64/sdk/lib/debug
    DESTDIR_SDK_WIN ~= s,/,\\,g

    QMAKE_POST_LINK += $$quote(if not exist $${DESTDIR_SDK_WIN} mkdir $${DESTDIR_SDK_WIN} $$escape_expand(\n\t))

    for(FILE,EXTRA_SDKFILES_WIN){
        QMAKE_POST_LINK += $$quote(cmd /c copy /y $${FILE} $${DESTDIR_SDK_WIN}$$escape_expand(\n\t))
    }

    export(QMAKE_POST_LINK)
}

CONFIG(release, debug|release): {
    EXTRA_BINFILES += $$quote($$_PRO_FILE_PWD_/../lib_x64/win/Release/$$1/release/*.dll)

    EXTRA_BINFILES_WIN = $${EXTRA_BINFILES}
    EXTRA_BINFILES_WIN ~= s,/,\\,g

    DESTDIR_WIN = $$_PRO_FILE_PWD_/../../bin/win64/release
    DESTDIR_WIN ~= s,/,\\,g

    QMAKE_POST_LINK += $$quote(if not exist $${DESTDIR_WIN} mkdir $${DESTDIR_WIN} $$escape_expand(\n\t))

    for(FILE,EXTRA_BINFILES_WIN){
        QMAKE_POST_LINK += $$quote(cmd /c copy /y $${FILE} $${DESTDIR_WIN}$$escape_expand(\n\t))
    }


    ###### copy pdb files
    EXTRA_PDBFILES += $$quote($$_PRO_FILE_PWD_/../lib_x64/win/Release/$$1/release/*.pdb)

    EXTRA_PDBFILES_WIN = $${EXTRA_PDBFILES}
    EXTRA_PDBFILES_WIN ~= s,/,\\,g

    DESTDIR_PDB_WIN = $$_PRO_FILE_PWD_/../../bin/win64/release-pdb
    DESTDIR_PDB_WIN ~= s,/,\\,g

    QMAKE_POST_LINK += $$quote(if not exist $${DESTDIR_PDB_WIN} mkdir $${DESTDIR_PDB_WIN} $$escape_expand(\n\t))

    for(FILE,EXTRA_PDBFILES_WIN){
        QMAKE_POST_LINK += $$quote(cmd /c copy /y $${FILE} $${DESTDIR_PDB_WIN}$$escape_expand(\n\t))
    }

    ###### copy sdk files
    EXTRA_SDKFILES += $$quote($$_PRO_FILE_PWD_/../lib_x64/win/Release/$$1/release/*.lib)

    EXTRA_SDKFILES_WIN = $${EXTRA_SDKFILES}
    EXTRA_SDKFILES_WIN ~= s,/,\\,g

    DESTDIR_SDK_WIN = $$_PRO_FILE_PWD_/../../bin/win64/sdk/lib/release
    DESTDIR_SDK_WIN ~= s,/,\\,g

    QMAKE_POST_LINK += $$quote(if not exist $${DESTDIR_SDK_WIN} mkdir $${DESTDIR_SDK_WIN} $$escape_expand(\n\t))

    for(FILE,EXTRA_SDKFILES_WIN){
        QMAKE_POST_LINK += $$quote(cmd /c copy /y $${FILE} $${DESTDIR_SDK_WIN}$$escape_expand(\n\t))
    }

    export(QMAKE_POST_LINK)
}

}

unix: {

CONFIG(debug, debug|release): {

    EXTRA_BINFILES += $$quote($$_PRO_FILE_PWD_/../lib_x64/lnx/Debug/$$1/*.so*)

    EXTRA_BINFILES_LNX = $${EXTRA_BINFILES}

    DESTDIR_LNX = $$_PRO_FILE_PWD_/../../bin/lnx/debug

    QMAKE_POST_LINK += $$quote(mkdir -p $${DESTDIR_LNX} $$escape_expand(\n\t))

    for(FILE,EXTRA_BINFILES_LNX){
        QMAKE_POST_LINK += $$quote(cp $${FILE} $${DESTDIR_LNX}$$escape_expand(\n\t))
    }

    ###### copy sdk files
    EXTRA_SDKFILES += $$quote($$_PRO_FILE_PWD_/../lib_x64/lnx/Debug/$$1/*.so*)

    EXTRA_SDKFILES_LNX = $${EXTRA_SDKFILES}

    DESTDIR_SDK_LNX = $$_PRO_FILE_PWD_/../../bin/lnx/sdk/lib/debug

    QMAKE_POST_LINK += $$quote(mkdir -p $${DESTDIR_SDK_LNX} $$escape_expand(\n\t))

    for(FILE,EXTRA_SDKFILES_LNX){
        QMAKE_POST_LINK += $$quote(cp $${FILE} $${DESTDIR_SDK_LNX}$$escape_expand(\n\t))
    }

    export(QMAKE_POST_LINK)
}

CONFIG(release, debug|release): {
    EXTRA_BINFILES += $$quote($$_PRO_FILE_PWD_/../lib_x64/lnx/Release/$$1/*.so*)

    EXTRA_BINFILES_LNX = $${EXTRA_BINFILES}

    DESTDIR_LNX = $$_PRO_FILE_PWD_/../../bin/lnx/release

    QMAKE_POST_LINK += $$quote(mkdir -p $${DESTDIR_LNX} $$escape_expand(\n\t))

    for(FILE,EXTRA_BINFILES_LNX){
        QMAKE_POST_LINK += $$quote(cp $${FILE} $${DESTDIR_LNX}$$escape_expand(\n\t))
    }

    ###### copy sdk files
    EXTRA_SDKFILES += $$quote($$_PRO_FILE_PWD_/../lib_x64/lnx/Release/$$1/*.so*)

    EXTRA_SDKFILES_LNX = $${EXTRA_SDKFILES}

    DESTDIR_SDK_LNX = $$_PRO_FILE_PWD_/../../bin/lnx/sdk/lib/release

    QMAKE_POST_LINK += $$quote(mkdir -p $${DESTDIR_SDK_LNX} $$escape_expand(\n\t))

    for(FILE,EXTRA_SDKFILES_LNX){
        QMAKE_POST_LINK += $$quote(cp $${FILE} $${DESTDIR_SDK_LNX}$$escape_expand(\n\t))
    }

    export(QMAKE_POST_LINK)
}

}

}
