// JavaScript Document

var myimages=new Array(
	"images/spacer.gif",			   
	"images/resume_snapshot.gif",
	"images/sunborn.jpg",
	"images/eg_icon.gif",
	"images/google_got_jokes.gif"
);

var mymovies=new Array(
    "sunbornmovie",
    "egpmovie"
);

var mytargets = new Array(
    "sunborntarget",
    "egptarget"
);

function getFlashHtml(movieName) {
    var strFlash = "<object id='flashmovie' classid='clsid:d27cdb6e-ae6d-11cf-96b8-444553540000' codebase='http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=10,0,0,0'";
    strFlash += "width='320' height='240' align='middle'>";
    strFlash += "<param name='allowScriptAccess' value='sameDomain' /><param name='allowFullScreen' value='false' /><param name='movie' value='" + movieName + "' />";
    strFlash += "<param name='play' value='true' /><param name='quality' value='high' /><param name='bgcolor' value='#ffffff' /><embed src='" + movieName + "' quality='high' bgcolor='#ffffff' width='320' height='240' play='true' swliveconnect='true' name='flashmovie' align='middle' allowscriptaccess='sameDomain' allowfullscreen='false' type='application/x-shockwave-flash' pluginspage='http://www.adobe.com/go/getflashplayer' />";
    strFlash += "</object>";

    return strFlash;
}

// Preload pics
function PreloadImages() {
    preImages = new Array();
    for (i = 0; i < myimages.length; i++) {
        preImages[i] = new Image();
        preImages[i].src = myimages[i];
    }
}

function getFlashMovieObject(movieName) {
    if (window.document[movieName]) {
        return window.document[movieName];
    }
    if (navigator.appName.indexOf("Microsoft Internet") == -1) {
        if (document.embeds && document.embeds[movieName])
            return document.embeds[movieName];
    }
    else // if (navigator.appName.indexOf("Microsoft Internet")!=-1)
    {
        return document.getElementById(movieName);
    }
}

function hidePreviousTarget() {
    if (lasttarget) {
        lasttarget.style.visibility = 'hidden';
    }

    document['targetimage'].style.visibility = 'hidden';
}

function showPic(i) {
    return;
    hidePreviousTarget();
    if (lastmovie)
        lastmovie.Rewind();
    
    if (document.images) {
        document['targetimage'].src = preImages[i].src;
        if (i > 0) {
            document['targetimage'].style.visibility = 'visible';
        }
    }
}

var lastmovie, lasttarget;
function showTarget(i) {
    return;
    hidePreviousTarget();

    if (i >= 0) {
        var targetObj = document.getElementById(mytargets[i]);
        targetObj.style.visibility = 'visible';

        var flashmovie = getFlashMovieObject(mymovies[i]);
        if (lastmovie != flashmovie)    // prevent rewinds if user rolls over the same link.
            flashmovie.Rewind();
            
        flashmovie.Play();

        lasttarget = targetObj;
        lastmovie = flashmovie;
    }
}
