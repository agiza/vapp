<div class="twelve columns">
  <?php
  require_once 'portfolio_filter.tpl.php';
  ?>
</div>

<?php if (!empty($nodes)): ?>
  <div class="twelve columns">

    <div class="row">

      <div id="container" class="clickable variable-sizes clearfix">

        <?php foreach ($nodes as $node) : ?>
          <?php
          $image_full = '';
          if (!empty($node->field_portfolio_image[LANGUAGE_NONE][0]['uri'])) {
            $image_full = file_create_url($node->field_portfolio_image[LANGUAGE_NONE][0]['uri']);
          }
          ?>
          <!-- Portfolio Item -->
          <div class="four columns element <?php print portfolio_format_terms('field_portfolio_category', $node); ?>">

            <div class="portfolio-item">
              <?php
              $image_uri = $node->field_portfolio_image[LANGUAGE_NONE][0]['uri'];
              $image_url = file_create_url($image_uri);
              $style_name = 'portfolio_item';
              $node_url = url('node/' . $node->nid);
              $node_title = $node->title;
              ?>
              <div class="portfolio-item-image image-overlay">    								
                <a class="titan-lb" data-titan-group="gallery" href="<?php print $image_full; ?>" title="<?php print $node->title; ?>">
                  <?php print theme('image_style', array('style_name' => $style_name, 'path' => $image_uri)); ?>
                  <span class="overlay-icon item-zoom"></span>
                </a>
              </div>

              <div class="portfolio-item-content">
                <h5 class="title"><a href="<?php print $node_url; ?>"><?php print $node_title; ?></a></h5> 
                <p><?php print custom_format_category_field('field_portfolio_category', $node); ?> </p>
              </div>

            </div>

          </div>
          <!-- End Portfolio Item --> 
        <?php endforeach; ?>


      </div>

    </div>

  </div>
<?php endif; ?>