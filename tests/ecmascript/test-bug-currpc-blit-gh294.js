/*
 *  Some problem code used in investigating GH-294.
 */

/*===
blitMask 11 52 123
done
===*/

function Image() {
}
Image.prototype.blitMask = function (x, y, color) {
    print('blitMask', x, y, color);
};
var myImage = new Image();

function CreateColor() { return 123; }

this.spriteset = {
    directions: [ { frames: [ { index: 0 } ] } ],
    images: [ myImage ],
};
this.directionID = 0;
this.frameID = 0;
this.xOff = 10;
this.yOff = 50;

this.blit = function(x, y, alpha) {
    alpha = alpha !== void null ? alpha : 255;

    this.spriteset.images[this.spriteset.directions[this.directionID].frames[this.frameID].index]
        .blitMask(x + this.xOff, y + this.yOff, CreateColor(255, 255, 255, alpha));
    //print("*munch*");
};

this.blit(1,2,3);
print('done');
