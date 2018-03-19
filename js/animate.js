// build time:Mon Mar 19 2018 16:00:11 GMT+0800 (CST)
"use strict";(function(e){e(document).ready(function(){var r=300;var a={opacity:1,top:0};function n(){var n=e.Deferred();var t=e(".animate");var i=t.length;t.each(function(t,o){e(o).delay(t*r).animate(a);i-1===t&&n.resolve()});return n.promise()}n()})})(jQuery);
//rebuild by neat 