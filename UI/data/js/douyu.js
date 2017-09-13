"use strict";

if (window.location.href.indexOf("login") > 0) {
    // TODO need login
} else if(window.location.href.indexOf("room/my") > 0) {
    // check cookie to see if user enables broadcast
    // if acf_own_room is 1, then broadcast is enabled
    if (document.cookie.indexOf('acf_own_room=1') != -1) {
        // check if already in living state, we can check rtmp_url selector
        if(document.querySelector('#rtmp_url') == null) {
            // TODO not in living
        } else {
            // get rtmp url and push code, set it back
            var url = document.querySelector('#rtmp_url').value;
            var key = document.querySelector('#rtmp_val').value;
            new QWebChannel(qt.webChannelTransport, function(channel) {
                var lp = channel.objects.lp
                lp.GrabLivePlatformInfo(url, key)
                lp.CloseWeb()
            })
        }
    } else {
        // TODO not enabled
    }
} else {
    window.location.href = "https://www.douyu.com/room/my"
}