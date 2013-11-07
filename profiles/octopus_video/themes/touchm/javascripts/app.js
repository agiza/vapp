

;
(function ($, window, undefined) {
  'use strict';

  var $doc = $(document),
  Modernizr = window.Modernizr;

  $(document).ready(function(){
    $.fn.foundationAlerts           ? $doc.foundationAlerts() : null;
    $.fn.foundationAccordion        ? $doc.foundationAccordion() : null;
    $.fn.foundationTooltips         ? $doc.foundationTooltips() : null;
    $('input, textarea').placeholder();
  
  
    $.fn.foundationButtons          ? $doc.foundationButtons() : null;
  
  
    $.fn.foundationNavigation       ? $doc.foundationNavigation() : null;
  
  
    $.fn.foundationTopBar           ? $doc.foundationTopBar() : null;
  
    $.fn.foundationCustomForms      ? $doc.foundationCustomForms() : null;
    $.fn.foundationMediaQueryViewer ? $doc.foundationMediaQueryViewer() : null;
  
    
    $.fn.foundationTabs             ? $doc.foundationTabs() : null;
    
  
  
    $("#featured").orbit();
  });
  

  // UNCOMMENT THE LINE YOU WANT BELOW IF YOU WANT IE8 SUPPORT AND ARE USING .block-grids
  // $('.block-grid.two-up>li:nth-child(2n+1)').css({clear: 'both'});
  // $('.block-grid.three-up>li:nth-child(3n+1)').css({clear: 'both'});
  // $('.block-grid.four-up>li:nth-child(4n+1)').css({clear: 'both'});
  // $('.block-grid.five-up>li:nth-child(5n+1)').css({clear: 'both'});

  // Hide address bar on mobile devices
  if (Modernizr.touch) {
    $(window).load(function () {
      setTimeout(function () {
        window.scrollTo(0, 1);
      }, 0);
    });
  }

})(jQuery, this);


// Slider Revolution




(function ($) {
  
  $(window).load(function() {
    jQuery('.mainslider').revolution(
    {
      delay:9000,
      startheight:490,
      startwidth:950,

      thumbWidth:100,
      thumbHeight:50,
      thumbAmount:4,

      onHoverStop:"on",
      hideThumbs:200,
      navigationType:"thumb",
      navigationStyle:"round",
      navigationArrows:"verticalcentered",

      touchenabled:"on",

      navOffsetHorizontal:0,
      navOffsetVertical:0,
      shadow:1,
      fullWidth:"off"
    });
  
  
    $(function() {
      $(".contentHover").hover(
        function() {
          $(this).children(".content").fadeTo(200, 0.25).end().children(".hover-content").fadeTo(200, 1).show();
        },
        function() {
          $(this).children(".content").fadeTo(200, 1).end().children(".hover-content").fadeTo(200, 0).hide();
        });
    });
		
        
  });
  // Flexi Slider



  <!-- Target sliders individually with different properties -->
  $(window).load(function() {
	  
    $('.simple-slider').flexslider({
      animation: "slide",
      slideshow: false,
      controlNav: false,
      smoothHeight: true,
      start: function(slider){
        $('body').removeClass('loading');
      }
    });
	
    $('.gallery-slider').flexslider({
      animation: "slide",
      controlNav: "thumbnails",
      start: function(slider){
        $('body').removeClass('loading');
      }
    });
 
    $('#main-slider').flexslider({
      animation: "slide",
      controlNav: false,
      start: function(slider){
        $('body').removeClass('loading');
      }
    });
    
    
  });


  $(document).ready(function(){
  
    var site_name = $('#logo h1');
    if(site_name.html() == 'Touchm'){
      site_name.html('Touch<span>M</span>');
    }
    $('.form-submit').addClass('medium button');
  
  });


  $(window).load(function(){
    
    var $container = $('#container');

    $container.isotope({
      itemSelector : '.element',
      // disable resizing
      resizable: false,
      // set columnWidth to a percentage of container width
      masonry: {
        columnWidth: $container.width() / 12
      }
    });
    
    // update columnWidth on window resize
    $(window).smartresize(function(){
      $container.isotope({
        // set columnWidth to a percentage of container width
        masonry: {
          columnWidth: $container.width() / 12
        }
      });
    });
      
    
    var $optionSets = $('#options .option-set'),
    $optionLinks = $optionSets.find('a');

    $optionLinks.click(function(){
      var $this = $(this);
      // don't proceed if already selected
      if ( $this.hasClass('selected') ) {
        return false;
      }
      var $optionSet = $this.parents('.option-set');
      $optionSet.find('.selected').removeClass('selected');
      $this.addClass('selected');
  
      // make option object dynamically, i.e. { filter: '.my-filter-class' }
      var options = {},
      key = $optionSet.attr('data-option-key'),
      value = $this.attr('data-option-value');
      // parse 'false' as false boolean
      value = value === 'false' ? false : value;
      options[ key ] = value;
      if ( key === 'layoutMode' && typeof changeLayoutMode === 'function' ) {
        // changes in layout modes need extra logic
        changeLayoutMode( $this, options )
      } else {
        // otherwise, apply new options
        $container.isotope( options );
      }
        
      return false;
    });
    
  });
  
  
})(jQuery);



	
