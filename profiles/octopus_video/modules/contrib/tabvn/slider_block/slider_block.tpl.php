<!-- Main Slider -->  
<div class="mainslider-container responsive" >
  <div class="mainslider" >
    <ul>

      <?php foreach ($nodes as $node): ?>

        <?php
        $transition = 'slotfade-horizontal';
        if (!empty($node->field_slider_transition[LANGUAGE_NONE][0]['value'])) {
          $transition = $node->field_slider_transition[LANGUAGE_NONE][0]['value'];
        }
		$image_field = field_get_items('node', $node, 'field_slider_image');
        $image_url = file_create_url($image_field[0]['uri']);
        $body = field_get_items('node', $node, 'body');

        if (!empty($body[0]['value'])) {
          $body_value =$body[0]['value'];
        }
        ?>
        <!-- SLIDE DOWN -->
        <li data-transition="slotfade-horizontal" data-slotamount="10" data-masterspeed="300"  data-thumb="<?php print $image_url; ?>">
          <img src="<?php print $image_url; ?>" alt="">
          <?php print $body_value; ?>
        </li>

      <?php endforeach; ?>

    </ul>
    <div class="tp-bannertimer"></div>
  </div>
</div>

<!-- End Main Slider -->