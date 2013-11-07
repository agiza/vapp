<?php
/**
 * @file views-view-unformatted.tpl.php
 * Default simple view template to display a list of rows.
 *
 * @ingroup views_templates
 */
?>

<div class="fractionslider-wrapper">
  <div class="fractionslider">
    <?php foreach ($rows as $id => $row): ?>
    <div class="slide <?php print $class; ?>"><?php print $row; ?></div>
    <?php endforeach; ?>
  </div>
</div>
<script type="text/javascript" charset="utf-8">
(function ($) {
  $(window).load(function(){
	$('.fractionslider').fractionSlider({
		'controls': 			<?php print $controls; ?>,
		'pager': 				<?php print $pager; ?>
	});
  });
})(jq190);
</script>
<script type="text/javascript" charset="utf-8">
(function ($) {
  $(window).load(function(){
	$(".fractionslider .slide").each(function() {
		var count = 1, pos = <?php print $space; ?>, step = 1, dirc = 1;
		var datain = ["left", "topLeft", "bottomLeft", "right", "topRight", "bottomRight", "top", "bottom"];
		var dir = datain[Math.floor(Math.random() * datain.length)];
		var dir2 = datain[Math.floor(Math.random() * datain.length)];
		$(this).children('.views-field').each(function() {
			$(this).attr({
				"data-step": step++,
				"data-position": pos*count++ + ",30",
				"data-in": dir,
				"data-out": dir2
				});
		});
		$(this).children('.views-field').attr({
			"data-speed": <?php print $dataspeed; ?>,
			"data-delay": <?php print $datadelay; ?>
		});
	});
  });
})(jQuery);
</script>

