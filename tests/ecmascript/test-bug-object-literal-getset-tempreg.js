// Reported here: https://github.com/svaarala/duktape/pull/1190#issuecomment-267533606

/*===
still here
function true true true
{getMusicFilename:{_func:true},load:{_func:true},fadeIn:{_func:true},fadeOut:{_func:true},update:{_func:true},musicPath:"~/music/",play:{_func:true},playSound:{_func:true},forceFinish:{_func:true},setMaxMusicVolume:{_func:true},setMaxSoundVolume:{_func:true},setVolume:{_func:true},stop:{_func:true},save:{_func:true},done:undefined,volume:undefined,svolume:undefined,mute:undefined}
===*/

var obj = {
    getMusicFilename: function() {},
    load: function() {},
    fadeIn: function() {},
    fadeOut: function() {},
    update: function() {},
    musicPath: "~/music/",
    play: function() {},
    playSound: function() {},
    forceFinish: function() {},
    setMaxMusicVolume: function() {},
    setMaxSoundVolume: function() {},
    setVolume: function() {},
    stop: function() {},
    save: function() {},
    get done() {},
    get volume() {},
    get svolume() {},
    get mute() {},
    set mute(value) {}
};

obj.setMaxSoundVolume();
print('still here');

var pd = Object.getOwnPropertyDescriptor(obj, 'setMaxSoundVolume');
print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable);

print(Duktape.enc('jx', obj));
