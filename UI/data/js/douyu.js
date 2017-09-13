"use strict";

function simplifyLoginPage() {
    document.getElementById("header").remove()
    document.getElementById("footer").remove()
    document.querySelector('.mainbody').setAttribute('style',
        "width: 300px;" +
        "min-width: 300px;" +
        "background-size: 0;" +
        "background: none;" +
        "height: auto;" +
        "margin: 0;" +
        "padding: 0;" +
        "left: 50%;")
    document.getElementById('loginbox').setAttribute('style', 'margin:0 auto;')
    document.querySelector('.hot-ad-con').remove()
    document.querySelector('.loginNormal').classList.remove('hide')
    document.querySelector('.scancode-login').remove()
    document.querySelector('.scanicon-switch-box').remove()
    document.querySelector('.third-text').remove()
    document.querySelector('.third-list.clearfix').remove()
    document.querySelector('.loginbox-noact.clearfix').remove()
    document.querySelector('.login-box-toptxt').remove()
    new QWebChannel(qt.webChannelTransport, function(channel) {
        var lp = channel.objects.lp
        document.querySelector('.wrapper.sign-con').setAttribute('style',
            'width:' + lp.m_pageWidth + 'px;')
    })
}

if (window.location.href.indexOf("login") > 0) {
    // simplify page
    simplifyLoginPage()
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