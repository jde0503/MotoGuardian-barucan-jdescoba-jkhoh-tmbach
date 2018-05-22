function initMap() {
        var map = new google.maps.Map(document.getElementById('map'), {
          zoom: 8,
          center: {lat: 32.880060, lng: -117.234014}
        });
        var geocoder = new google.maps.Geocoder;
        var infowindow = new google.maps.InfoWindow;

        window.onload = function() {
          geocodeLatLng(geocoder, map, infowindow);
        }
      }

function geocodeLatLng(geocoder, map, infowindow) {
  // var input = document.getElementById('latlng').value;
  var input = '32.880060,-117.234014';
  var latlngStr = input.split(',', 2);
  var latlng = {lat: parseFloat(latlngStr[0]), lng: parseFloat(latlngStr[1])};
  geocoder.geocode({'location': latlng}, function(results, status) {
    if (status === 'OK') {
      if (results[0]) {
        map.setZoom(11);
        var marker = new google.maps.Marker({
          position: latlng,
          map: map
        });
        infowindow.setContent(results[0].formatted_address);
        infowindow.open(map, marker);
      } else {
        window.alert('No results found');
      }
    } else {
      window.alert('Geocoder failed due to: ' + status);
    }
  });
}
$("#map").css("width", 600).css("height", 400);
$("#lock").css("width", 200).css("height", 200);

$("#lock").click(function() { 
       var _this = $(this);
       var current = _this.attr("src");
       var swap = _this.attr("data-swap");     
     _this.attr('src', swap).attr("data-swap",current);   
});  

new DG.OnOffSwitch({
    el: '#on-off-switch-custom',
    height: 70,
    trackColorOn:'red',
    trackColorOff:'#666',
    trackBorderColor:'black',
    textColorOff:'#fff',
    textOn:'ARMED',
    textOff:'Disarmed'
});

