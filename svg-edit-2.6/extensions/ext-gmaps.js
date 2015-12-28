/*
 * ext-gmap.js
 * Licensed under the MIT License
 * Copyright(c) 2010 Corey Auger
 */

function SVGOverlay(map) {

    // Now initialize all properties.
    this.map_ = map;

    // We define a property to hold the image's
    // div. We'll actually create this div
    // upon receipt of the add() method so we'll
    // leave it null for now.
    this.div_ = null;

    // Explicitly call setMap() on this overlay
    this.setMap(map);
}

SVGOverlay.prototype = new google.maps.OverlayView();
SVGOverlay.prototype.onAdd = function() {

    // Note: an overlay's receipt of onAdd() indicates that
    // the map's panes are now available for attaching
    // the overlay to the map via the DOM.

    // Create the DIV and set some basic attributes.
    var div = document.createElement('div');
    div.id = 'gmapSvgOverlay';
    div.style.border = "1px solid red";
    div.style.borderWidth = "1px";

    div.style.position = "absolute";

    // Set the overlay's div_ property to this DIV
    this.div_ = div;

    // We add an overlay to a map via one of the map's panes.
    // We'll add this overlay to the overlayImage pane.
    var panes = this.getPanes();
    panes.overlayLayer.appendChild(div);
};
SVGOverlay.prototype.draw = function() {

    // Size and position the overlay. We use a southwest and northeast
    // position of the overlay to peg it to the correct position and size.
    // We need to retrieve the projection from this overlay to do this.
    var overlayProjection = this.getProjection();

    // Retrieve the southwest and northeast coordinates of this overlay
    // in latlngs and convert them to pixels coordinates.
    // We'll use these coordinates to resize the DIV.
   // var sw = overlayProjection.fromLatLngToDivPixel(this.bounds_.getSouthWest());
    //var ne = overlayProjection.fromLatLngToDivPixel(this.bounds_.getNorthEast());


    // Resize the image's DIV to fit the indicated dimensions.
    var div = this.div_;
 //   div.style.left = sw.x + 'px';
 ///   div.style.top = ne.y + 'px';
 //   div.style.width = (ne.x - sw.x) + 'px';
 //   div.style.height = (sw.y - ne.y) + 'px';
};
SVGOverlay.prototype.onRemove = function() {
    this.div_.parentNode.removeChild(this.div_);
    this.div_ = null;
};


$(window).resize(function(){
    //console.log('resize: ' + $('#svgcanvas').innerWidth());
    $('#map-canvas').width($(window).innerWidth())
                    .height($(window).innerHeight());
});

$(function(){
    $('#workarea').prepend('<div id="map-canvas" style="position:absolute;"></div>')
                    .css('line-height','normal');       // we need to override this css (it totally messes with the map)
    $(window).trigger('resize');
});


svgEditor.addExtension("Google Maps", function() {

    var _id = "gmap";
    var transformOrg = [0, 0];


    var transformStack = [];
    var svgZoomed = true;
    // remove the white background SVG in the middle of the screen...
    $("rect").first().remove();
    var curZoom = 15;
    svgEditor.startMapZoom = curZoom;
    svgEditor.mapLastZoom = curZoom;
    var zoomFac = 1;
    var mapOptions = {
        center: new google.maps.LatLng(49.303938, -123.144245),
        zoom: svgEditor.startMapZoom,
        mapTypeId: google.maps.MapTypeId.ROADMAP,
        disableDefaultUI: true // a way to quickly hide all controls
    };
    svgEditor.map = new google.maps.Map(document.getElementById("map-canvas"),mapOptions);
    /*
    google.maps.event.addListener(svgEditor.map, "bounds_changed", function() {
        // send the new bounds back to your server
        var bb = svgEditor.map.getBounds();
        var proj = svgEditor.map.getProjection();
        var wc = proj.fromLatLngToPoint(bb.getNorthEast());
        console.log('bounds: ' + bb);
    });
    */


    function overlayTransform(){
        if( svgCanvas.getMode() == _id ){
            var matrix = $('#gmapSvgOverlay').parent().parent().parent().css('-webkit-transform');
            if( matrix && matrix.indexOf('matrix') >=0 ){
                var parts = matrix.replace(/matrix/g,'').split(',');
                var x = parseInt(parts[parts.length-2]);
                var y = parseInt(parts[parts.length-1]);
                $('#gmapSvgOverlay').css('left', -(x)+'px');
                $('#gmapSvgOverlay').css('top', -(y)+'px');
                svgcontent[0].setAttribute('x',-transformOrg[0]+(x));
                svgcontent[0].setAttribute('y',-transformOrg[1]+(y));

               // console.log( transformOrg[0] + " " + x);
            }
        }
    }




    function svgTransform(){
        var matrix = $('#gmapSvgOverlay').parent().parent().parent().css('-webkit-transform');
        var transformOriginal = $('#gmapSvgOverlay').parent().parent().parent().css('-webkit-transform-origin');
        if( transformOriginal ){
            var txy =  transformOriginal.split(' ');
            var tx = parseInt(txy[0]);
            var ty = parseInt(txy[1]);
            if( matrix && matrix.indexOf('matrix') >=0 ){
                var parts = matrix.replace(/matrix/g,'').split(',');
                var x = parseInt(parts[parts.length-2]);
                var y = parseInt(parts[parts.length-1]);

                if(svgZoomed){
                    // ok this is the correct transform... we take the new translation + 2 times the old one becuse we double the zoom
                  //  transformOrg[0] = (-tx+(x)) + (zoomScaler*transformOrg[0]);
                //    transformOrg[1] = (-ty+(y)) + (zoomScaler*transformOrg[1]);
                //    console.log('** ' + (-tx+(x)) + ' + ' + zoomScaler + '*'+tmp+ '(=)'+ (zoomScaler*tmp) +' = '+ transformOrg[0]);
                    transformStack.push({ tx:(-tx+x), ty:(-ty+y), x:x, y:y, z:svgEditor.mapLastZoom, dir:svgEditor.map.getZoom()-svgEditor.mapLastZoom  });
                    svgEditor.mapLastZoom = svgEditor.map.getZoom();
                    svgZoomed = false;
                }
                console.log("ZOOM: " + svgEditor.map.getZoom() );
                var accum = [0, 0];
                for(var i=0; i<transformStack.length; i++){
                    //var dir = svgEditor.map.getZoom() - transformStack[i].z;
                    var dir = transformStack[i].dir;
                    if( dir < 0 ){
                        zFac = Math.pow(2,svgEditor.map.getZoom()-transformStack[i].z);
                        accum[0] -= (transformStack[i].tx*zFac);
                        accum[1] -= (transformStack[i].ty*zFac);
                        console.log('z('+transformStack[i].z+') '+zFac +' * ' +  (-transformStack[i].tx) +' = '+(-transformStack[i].tx*zFac)+ '     total: ' + accum[0] );
                    }
                    else{
                        zFac = Math.pow(2,svgEditor.map.getZoom()-transformStack[i].z-1);
                        accum[0] += (transformStack[i].tx*zFac);
                        accum[1] += (transformStack[i].ty*zFac);
                        console.log('z('+transformStack[i].z+') '+zFac +' * ' +  transformStack[i].tx +' = '+(transformStack[i].tx*zFac)+ '     total: ' + accum[0] );
                    }
                }
                svgcontent.setAttribute('x', accum[0]+x );
                svgcontent.setAttribute('y', accum[1]+y );
               // svgcontent.setAttribute('x', transformOrg[0]+(x) );
               // svgcontent.setAttribute('y', transformOrg[1]+(y) );
            }
        }
    }

    google.maps.event.addListener(svgEditor.map, 'dragend', function() {
        svgTransform();
    });

//    google.maps.event.addListener(svgEditor.map, 'drag', function() {
 //       overlayTransform();
//        svgTransform();
//    });

    google.maps.event.addListener(svgEditor.map, 'zoom_changed', function() {
        var zoomLevel = svgEditor.map.getZoom();
        svgZoomed = true;
        if( zoomLevel - curZoom > 0){
            var res = svgCanvas.getResolution();
            var w = $(window).innerWidth();
            var h = $(window).innerHeight();
            zoomFac *= 2;
         //   $('#gmapSvgOverlay').html('');
            svgcontent.setAttribute('viewBox','0 0 '+(w/zoomFac)+' '+(h/zoomFac));
            zoomScaler = 2;
         //   $('#gmapSvgOverlay').html($('#svgroot').clone());
        }else{
            var res = svgCanvas.getResolution();
            var w = $(window).innerWidth();
            var h = $(window).innerHeight();
            zoomFac /= 2;
        //    $('#gmapSvgOverlay').html('');
            svgcontent.setAttribute('viewBox','0 0 '+(w/zoomFac)+' '+(h/zoomFac));
         //   $('#gmapSvgOverlay').html($('#svgroot').clone());
        }
        setTimeout(function(){svgTransform();}, 500);  // TODO: need an event like "zoom_end"
        curZoom =  zoomLevel;
    });

    overlay = new SVGOverlay(svgEditor.map);
    // hide svg.. so map will get events

    // this was not needed and now it seems to be to kick things off
   // $('#gmapSvgOverlay').html($('#svgroot').clone());
    $('#gmapSvgOverlay').append('<div style="background-color:red;width: 100px;height: 100px;"></div>');
    $('#svgcanvas').hide().css('border','1px solid red');

    // NOTE: SVG Edit should have an event that tells you "when" your tool has been selected..
    // thus allowing you to init state... for now we hack around this..
    $('#tools_left').on('click','div',function(){
        if($(this).attr('id') == _id){
            // hack to drag...
         //   svgcontent.setAttribute('x',0);
         //   svgcontent.setAttribute('y',0);

           // $('#gmapSvgOverlay').parent().parent().parent().css('-webkit-transform','matrix(1, 0, 0, 1, 0, 0)');

          //  $('#gmapSvgOverlay').html($('#svgroot').clone());
            $('#gmapSvgOverlay').append('<div style="border: 3px dashed red;width: 1000px;height: 1000px;"></div>');
            // hide svg.. so map will get events
            $('#svgcanvas').hide();
            //overlayTransform();
        }else{
            // show svg .. no more map events.. only svg!
            $('#gmapSvgOverlay').html('');
            $('#gmapSvgOverlay').append('<div style="border: 3px dashed red;width: 1000px;height: 1000px;"></div>');
            $('#svgcanvas').show();
           // svgCanvas.setResolution($(window).innerWidth(), $(window).innerHeight());
            svgCanvas.setResolution($(window).innerWidth(), $(window).innerHeight());
            svgTransform();
        }
    });

		return {
			name: "Google Maps",
			// For more notes on how to make an icon file, see the source of
			// the hellorworld-icon.xml
			svgicons: "/javascripts/svg-edit-2.6/extensions/gmap-icon.xml",
			
			// Multiple buttons can be added in this array
			buttons: [{
				// Must match the icon ID in helloworld-icon.xml
				id: _id,
				
				// This indicates that the button will be added to the "mode"
				// button panel on the left side
				type: "mode", 
				
				// Tooltip text
				title: "Google Maps SVG extension",
				
				// Events
				events: {
					'click': function() {
						// The action taken when the button is clicked on.
						// For "mode" buttons, any other button will 
						// automatically be de-pressed.
						svgCanvas.setMode(_id);
					}
				}
			}],
			// This is triggered when the main mouse button is pressed down 
			// on the editor canvas (not the tool panels)
			mouseDown: function() {
				// Check the mode on mousedown
				if(svgCanvas.getMode() == _id) {
				    // The returned object must include "started" with
					// a value of true in order for mouseUp to be triggered
					return {started: true};
				}
			},

            mouseMove: function(){
                if( svgCanvas.getMode() == _id){
                    console.log('mouse move');
                }
            },
			
			// This is triggered from anywhere, but "started" must have been set
			// to true (see above). Note that "opts" is an object with event info
			mouseUp: function(opts) {
				// Check the mode on mouseup
				if(svgCanvas.getMode() == _id) {
					var zoom = svgCanvas.getZoom();
					
					// Get the actual coordinate by dividing by the zoom value
					var x = opts.mouse_x / zoom;
					var y = opts.mouse_y / zoom;
					
					var text = "Hello World!\n\nYou clicked here: " 
						+ x + ", " + y;
						
					// Show the text using the custom alert function
					$.alert(text);
				}
			}
		};
});

