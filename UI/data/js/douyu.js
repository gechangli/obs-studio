"use strict";

if (window.location.href.indexOf("login") > 0) {
    // TODO need login
} elseif(window.location.href.indexOf("room/my") > 0) {
    // check cookie to see if user enables broadcast
    // if acf_own_room is 1, then broadcast is enabled
    if (document.cookie.indexOf('acf_own_room=1') != -1) {
        // check if already in living state, we can check rtmp_url selector
        if($$('#rtmp_url').length == 0) {
            // TODO not in living
        } else {
            // get rtmp url and push code, set it back
            var url = $('#rtmp_url').val();
            var key = $('#rtmp_val').val();
            new QWebChannel(qt.webChannelTransport, function(channel) {
                var lp = channel.objects.lp
                lp.GrabLivePlatformInfo(url, key)
            })
        }
    } else {
        // TODO not enabled
    }
}