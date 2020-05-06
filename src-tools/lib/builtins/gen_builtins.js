/*
 *  Generate initialization data for built-in strings and objects.
 *
 *  Supports two different initialization approaches:
 *
 *    1. Bit-packed format for unpacking strings and objects during
 *       heap or thread init into RAM-based structures.  This is the
 *       default behavior.
 *
 *    2. Embedding strings and/or objects into a read-only data section
 *       at compile time.  This is useful for low memory targets to reduce
 *       memory usage.  Objects in data section will be immutable.
 *
 *  Both of these have practical complications like endianness differences,
 *  pointer compression variants, object property table layout variants,
 *  and so on.  Multiple #if defined()'d initializer sections are emitted
 *  to cover all supported alternatives.
 *
 *  Based on genbuiltins.py.
 */

'use strict';

const { loadMetadata, augmentMetadata } = require('./metadata/load');
const { prettyPrintMetadata } = require('./metadata/pretty');
const { GenerateC } = require('../util/generate_c');
const {
    generateRamStringInitDataBitpacked,
    getRamobjNativeFuncMaps,
    generateRamObjectInitDataBitpacked,
    emitRamStringHeader,
    emitRamStringInitData,
    emitRamObjectNativeFuncArray,
    emitRamObjectInitData,
    emitRamObjectNativeFuncDeclarations,
    emitRamObjectNativeFuncArrayDeclaration,
    emitRamObjectHeader,
    emitRamObjectInitDataDeclaration
} = require('./ram_initdata');
const {
    emitStringsSource,
    emitObjectsSource,
    emitStridxDefinesHeader,
    emitStringsHeader,
    emitNativeFunctionDeclarationsHeader,
    emitObjectsHeader
} = require('./rom_initdata');
const {
    emit32BitPtrCheck,
    emitInitializerTypesAndMacrosSource,
    emitRefcountInitMacro
} = require('./initdata/object_initializers');
const { ROMPTR_FIRST } = require('./rom_initdata');
const { assert } = require('../util/assert');
const { jsonDeepClone } = require('../util/clone');

function emitBuiltinsSource(args) {
    var gcSrc = args.gcSrc;
    var romMeta = args.romMeta;
    var ramStrInitData = args.ramStrInitData;
    var ramNativeFuncs = args.ramNativeFuncs;
    var ramObjDataLe = args.ramObjDataLe;
    var ramObjDataBe = args.ramObjDataBe;
    var ramObjDataMe = args.ramObjDataMe;
    var biStrMap;

    gcSrc.emitLine('#include "duk_internal.h"');
    gcSrc.emitLine('');
    emit32BitPtrCheck(gcSrc);
    emitRefcountInitMacro(gcSrc);

    gcSrc.emitLine('#if defined(DUK_USE_ROM_STRINGS)');
    ({ biStrMap } = emitStringsSource(gcSrc, romMeta));
    emitInitializerTypesAndMacrosSource(gcSrc);
    emitObjectsSource(gcSrc, romMeta, biStrMap);
    gcSrc.emitLine('#else  /* DUK_USE_ROM_STRINGS */');
    emitRamStringInitData(gcSrc, ramStrInitData);
    gcSrc.emitLine('#endif  /* DUK_USE_ROM_STRINGS */');
    gcSrc.emitLine('');

    gcSrc.emitLine('#if defined(DUK_USE_ROM_OBJECTS)');
    gcSrc.emitLine('#if !defined(DUK_USE_ROM_STRINGS)');
    gcSrc.emitLine('#error DUK_USE_ROM_OBJECTS requires DUK_USE_ROM_STRINGS');
    gcSrc.emitLine('#endif');
    gcSrc.emitLine('#if defined(DUK_USE_HSTRING_ARRIDX)');
    gcSrc.emitLine('#error DUK_USE_HSTRING_ARRIDX is currently incompatible with ROM built-ins');
    gcSrc.emitLine('#endif');
    gcSrc.emitLine('#else  /* DUK_USE_ROM_OBJECTS */');
    emitRamObjectNativeFuncArray(gcSrc, ramNativeFuncs);
    gcSrc.emitLine('#if defined(DUK_USE_DOUBLE_LE)');
    emitRamObjectInitData(gcSrc, ramObjDataLe);
    gcSrc.emitLine('#elif defined(DUK_USE_DOUBLE_BE)')
    emitRamObjectInitData(gcSrc, ramObjDataBe);
    gcSrc.emitLine('#elif defined(DUK_USE_DOUBLE_ME)')
    emitRamObjectInitData(gcSrc, ramObjDataMe);
    gcSrc.emitLine('#else')
    gcSrc.emitLine('#error invalid endianness defines')
    gcSrc.emitLine('#endif')
    gcSrc.emitLine('#endif  /* DUK_USE_ROM_OBJECTS */');
}

function emitBuiltinsHeader(args) {
    var gcHdr = args.gcHdr;
    var romMeta = args.romMeta;
    var ramMeta = args.ramMeta;
    var ramStrInitData = args.ramStrInitData;
    var ramStrMaxLen = args.ramStrMaxLen;
    var ramNativeFuncs = args.ramNativeFuncs;
    var ramObjDataLe = args.ramObjDataLe;
    var ramObjDataBe = args.ramObjDataBe;
    var ramObjDataMe = args.ramObjDataMe;

    gcHdr.emitLine('#if !defined(DUK_BUILTINS_H_INCLUDED)');
    gcHdr.emitLine('#define DUK_BUILTINS_H_INCLUDED');
    gcHdr.emitLine('');

    gcHdr.emitLine('#if defined(DUK_USE_ROM_STRINGS)');
    emitStridxDefinesHeader(gcHdr, romMeta);
    emitStringsHeader(gcHdr, romMeta);
    gcHdr.emitLine('#else  /* DUK_USE_ROM_STRINGS */');
    emitStridxDefinesHeader(gcHdr, ramMeta);
    emitRamStringHeader(gcHdr, ramStrInitData, ramStrMaxLen);
    gcHdr.emitLine('#endif  /* DUK_USE_ROM_STRINGS */');
    gcHdr.emitLine('');

    gcHdr.emitLine('#if defined(DUK_USE_ROM_OBJECTS)');
    // Currently DUK_USE_ROM_PTRCOMP_FIRST must match our fixed
    // define, and the two must be updated in sync.  Catch any
    // mismatch to avoid difficult to diagnose errors.
    gcHdr.emitLine('#if !defined(DUK_USE_ROM_PTRCOMP_FIRST)');
    gcHdr.emitLine('#error missing DUK_USE_ROM_PTRCOMP_FIRST define');
    gcHdr.emitLine('#endif');
    gcHdr.emitLine('#if (DUK_USE_ROM_PTRCOMP_FIRST != ' + ROMPTR_FIRST + 'L)');
    gcHdr.emitLine('#error DUK_USE_ROM_PTRCOMP_FIRST must match ROMPTR_FIRST in duktool.js (' + ROMPTR_FIRST + '), update manually and re-dist');
    gcHdr.emitLine('#endif');
    emitNativeFunctionDeclarationsHeader(gcHdr, romMeta);
    emitObjectsHeader(gcHdr, romMeta);
    gcHdr.emitLine('#else  /* DUK_USE_ROM_OBJECTS */');
    emitRamObjectNativeFuncDeclarations(gcHdr, ramNativeFuncs);
    emitRamObjectNativeFuncArrayDeclaration(gcHdr, ramNativeFuncs);
    emitRamObjectHeader(gcHdr, ramMeta);
    gcHdr.emitLine('#if defined(DUK_USE_DOUBLE_LE)');
    emitRamObjectInitDataDeclaration(gcHdr, ramObjDataLe);
    gcHdr.emitLine('#elif defined(DUK_USE_DOUBLE_BE)');
    emitRamObjectInitDataDeclaration(gcHdr, ramObjDataBe);
    gcHdr.emitLine('#elif defined(DUK_USE_DOUBLE_ME)');
    emitRamObjectInitDataDeclaration(gcHdr, ramObjDataMe);
    gcHdr.emitLine('#else')
    gcHdr.emitLine('#error invalid endianness defines')
    gcHdr.emitLine('#endif')
    gcHdr.emitLine('#endif  /* DUK_USE_ROM_OBJECTS */');
    gcHdr.emitLine('');
    gcHdr.emitLine('#endif  /* DUK_BUILTINS_H_INCLUDED */');
}

function generateBuiltins(args) {
    var usedStridxEtcMeta = args.usedStridxEtcMeta;
    var dukVersion = args.dukVersion;
    var activeOpts = args.activeOpts;
    var romAutoLightfunc = args.romAutoLightfunc;
    var userBuiltinFiles = args.userBuiltinFiles;
    var objectsMetadataFile = args.objectsMetadataFile;
    var stringsMetadataFile = args.stringsMetadataFile;

    // Read in metadata files, normalizing and merging as necessary.

    var ramMeta = loadMetadata({
        stringsMetadataFilename: stringsMetadataFile,
        objectsMetadataFilename: objectsMetadataFile,
        userBuiltinFiles,
        usedStridxEtcMeta,
        activeOpts,
        romBuild: false,
        dukVersion
    });
    var ramMetaUnaugmented = jsonDeepClone(ramMeta);
    var prettyRamMeta = prettyPrintMetadata(ramMetaUnaugmented);
    augmentMetadata(ramMeta);

    var romMeta = loadMetadata({
        stringsMetadataFilename: stringsMetadataFile,
        objectsMetadataFilename: objectsMetadataFile,
        userBuiltinFiles,
        usedStridxEtcMeta,
        activeOpts,
        romBuild: true,
        romAutoLightfunc,
        dukVersion
    });
    var romMetaUnaugmented = jsonDeepClone(romMeta);
    var prettyRomMeta = prettyPrintMetadata(romMetaUnaugmented);
    augmentMetadata(romMeta);

    let ramStrData, ramStrMaxLen;
    let nativeFuncs, natfuncNameToNatidx;
    let ramObjDataLe, ramObjDataBe, ramObjDataMe;
    ({ data: ramStrData, maxLen: ramStrMaxLen } = generateRamStringInitDataBitpacked(ramMeta));
    ({ nativeFuncs, natfuncNameToNatidx } = getRamobjNativeFuncMaps(ramMeta));
    ({ data: ramObjDataLe } = generateRamObjectInitDataBitpacked(ramMeta, nativeFuncs, natfuncNameToNatidx, 'little'));
    ({ data: ramObjDataBe } = generateRamObjectInitDataBitpacked(ramMeta, nativeFuncs, natfuncNameToNatidx, 'big'));
    ({ data: ramObjDataMe } = generateRamObjectInitDataBitpacked(ramMeta, nativeFuncs, natfuncNameToNatidx, 'mixed'));
    assert(ramObjDataLe.length === ramObjDataBe.length);
    assert(ramObjDataLe.length === ramObjDataMe.length);

    // Create source and header files.

    var gcSrc = new GenerateC();
    emitBuiltinsSource({
        gcSrc,
        romMeta,
        ramStrInitData: ramStrData,
        ramNativeFuncs: nativeFuncs,
        ramObjDataLe,
        ramObjDataBe,
        ramObjDataMe
    });  // duk_builtins.c

    var gcHdr = new GenerateC();
    emitBuiltinsHeader({
        gcHdr,
        romMeta,
        ramMeta,
        ramStrInitData: ramStrData,
        ramStrMaxLen: ramStrMaxLen,
        ramNativeFuncs: nativeFuncs,
        ramObjDataLe,
        ramObjDataBe,
        ramObjDataMe
    });  // duk_builtins.h

    return {
        preparedRomMetadata: romMeta,
        unaugmentedRomMetadata: romMetaUnaugmented,
        prettyRomMetadata: prettyRomMeta,
        preparedRamMetadata: ramMeta,
        unaugmentedRamMetadata: ramMetaUnaugmented,
        prettyRamMetadata: prettyRamMeta,
        sourceString: gcSrc.getString(),
        headerString: gcHdr.getString()
    };
}
exports.generateBuiltins = generateBuiltins;
