// build time:Mon Mar 19 2018 18:07:25 GMT+0800 (CST)
"use strict";(function(e){e(document).ready(function(){var a=300;var r={opacity:1,top:0};function t(){var r=e.Deferred();var t=e(".animate");var n=t.length;t.each(function(t,c){e(c).delay(t*a).addClass("active");n-1===t&&r.resolve()});return r.promise()}t()})})(jQuery);
//rebuild by neat 