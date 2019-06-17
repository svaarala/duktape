/*
 *  https://github.com/svaarala/duktape/issues/2024
 *
 *  Slightly modified to make the expect string clean, while keeping the
 *  repro effective.
 */

/*===
done
===*/

function test ( ) {
    var func = function foo ( a , b , c ) { print ( a , b , c ) ; } ;
    func = function foo ( id_0, id_1 , id_2 , id_3 , id_4 , id_5 , id_6 , id_7 , id_8 , id_9 , id_10 , id_11 , id_12 , id_13 , id_14 , id_15 , id_16 , id_17 , id_18 , id_19 , id_20 , id_21 , id_22 , id_23 , id_24 , id_25 , id_26 , id_27 , id_28 , id_29 , id_30 , id_31 , id_32 , id_33 , id_34 , id_35 , id_36 , id_37 , id_38 , id_39 , id_40 , id_41 , id_42 , id_43 , id_44 , id_45 , id_46 , id_47 , id_48 , id_49 , id_50 , id_51 , id_52 , id_53 , id_54 , id_55 , id_56 , id_57 , id_58 , id_59 , id_60 , id_61 , id_62 , id_63 , id_64 , id_65 , id_66 , id_67 , id_68 , id_69 , id_70 , id_71 , id_72 , id_73 , id_74 , id_75 , id_76 , id_77 , id_78 , id_79 , id_80 , id_81 , id_82 , id_83 , id_84 , id_85 , id_86 , id_87 , id_88 , id_89 , id_90 , id_91 , id_92 , id_93 , id_94 , id_95 , id_96 , id_97 , id_98 , id_99 , id_100 , id_101 , id_102 , id_103 , id_104 , id_105 , id_106 , id_107 , id_108 , id_109 , id_110 , id_111 , id_112 , id_113 , id_114 , id_115 , id_116 , id_117 , id_118 , id_119 , id_120 , id_121 , id_122 , id_123 , id_124 , id_125 , id_126 , id_127 , id_128 , id_129 , id_130 , id_131 , id_132 , id_133 , id_134 , id_135 , id_136 , id_137 , id_138 , id_139 , id_140 , id_141 , id_142 , id_143 , id_144 , id_145 , id_146 , id_147 , id_148 , id_149 , id_150 , id_151 , id_152 , id_153 , id_154 , id_155 , id_156 , id_157 , id_158 , id_159 , id_160 , id_161 , id_162 , id_163 , id_164 , id_165 , id_166 , id_167 , id_168 , id_169 , id_170 , id_171 , id_172 , id_173 , id_174 , id_175 , id_176 , id_177 , id_178 , id_179 , id_180 , id_181 , id_182 , id_183 , id_184 , id_185 , id_186 , id_187 , id_188 , id_189 , id_190 , id_191 , id_192 , id_193 , id_194 , id_195 , id_196 , id_197 , id_198 , id_199 , id_200 , id_201 , id_202 , id_203 , id_204 , id_205 , id_206 , id_207 , id_208 , id_209 , id_210 , id_211 , id_212 , id_213 , id_214 , id_215 , id_216 , id_217 , id_218 , id_219 , id_220 , id_221 , id_222 , id_223 , id_224 , id_225 , id_226 , id_227 , id_228 , id_229 , id_230 , id_231 , id_232 , id_233 , id_234 , id_235, id_236 , id_237 , id_238 , id_239 ) {
        Math.cos ( 'arg239:' , id_104 ) ;
    };
    func.apply ( null , [ 0 ] );
    func = function ( a , b , c , d ) { print ( typeof id_19 ) ; } ;
    func = function ( a , b , c , d , e ) { function inner ( ) { print ( 'inner' ) } ; } ;
    test ( ) ;
    func = function ( a , b , c ) { print ( eval ( '"aiee"' ) ) ; } ;
}
try { test ( ) ; } catch ( e ) { /* print(e); */ }
print('done');
