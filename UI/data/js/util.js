"use strict";

function getCookie(name)  {
    var arr, reg = new RegExp("(^| )" + name + "=([^;]*)(;|$)");
    if(arr = document.cookie.match(reg))
        return unescape(arr[2]);
    else
        return null;
}

function queryCss(o, key) {
    return o.currentStyle? o.currentStyle[key] : document.defaultView.getComputedStyle(o, false)[key];
}