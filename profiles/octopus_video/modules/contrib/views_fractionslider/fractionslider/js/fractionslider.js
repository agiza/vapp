/**
* @file
* jQuery fractionslider
*/
(function ($) {
 Drupal.behaviors.fractionslider = {
  attach: function (context, settings) {
	$(window).load(function(){
	$('.fractionslider').fractionSlider({
		'fullWidth': 			true,
		'controls': 			false,
		'pager': 				false
	});
	});
  }
 };
})(jq190);