
<div class="container region1wrap">  

  <!-- Top Header -->
  <div class="row top_header">

    <div class="six columns">
      <?php print render($page['top_nav']); ?>
      <?php print render($page['login_box']); ?>
    </div>

    <div class="six columns">
      <?php
      $twitter_url = theme_get_setting('twitter_url', 'touchm');
      $facebook_url = theme_get_setting('facebook_url', 'touchm');
      $google_plus_url = theme_get_setting('google_plus_url', 'touchm');
      $linkedin_url = theme_get_setting('linkedin_url', 'touchm');
      $flickr_url = theme_get_setting('flickr_url', 'touchm');
      $vimeo_url = theme_get_setting('vimeo_url', 'touchm');
      $rss_url = theme_get_setting('rss_url', 'touchm');

      $theme_path = base_path() . path_to_theme();
      ?>
      <ul class="top_social">
        <li><a class="has-tipsy" href="<?php print $twitter_url; ?>" title="Twitter"><img src="<?php print $theme_path; ?>/images/social/Twitter.png" alt="Twitter"></a></li>
        <li><a class="has-tipsy" href="<?php print $facebook_url; ?>" title="Facebook"><img src="<?php print $theme_path; ?>/images/social/Facebook.png" alt="Facebook"></a>
        <li><a class="has-tipsy" href="<?php print $google_plus_url ?>" title="Google"><img src="<?php print $theme_path; ?>/images/social/Google.png" alt="Google+"></a></li>
        <li><a class="has-tipsy" href="<?php print $linkedin_url; ?>" title="LinkedIn"><img src="<?php print $theme_path; ?>/images/social/LinkedIn.png" alt="LinkedIn"></a></li>
        <li><a class="has-tipsy" href="<?php print $flickr_url; ?>" title="Flickr"><img src="<?php print $theme_path; ?>/images/social/Flickr.png" alt="Flickr"></a></li>
        <li><a class="has-tipsy" href="<?php print $vimeo_url; ?>" title="Vimeo"><img src="<?php print $theme_path; ?>/images/social/Vimeo.png" alt="Vimeo"></a></li>
        <li><a class="has-tipsy" href="<?php print $rss_url; ?>" title="RSS"><img src="<?php print $theme_path; ?>/images/social/RSS.png" alt="RSS"></a></li>
      </ul>

    </div>

  </div>
  <!-- End Top Header -->

</div>

<!-- End Region 1 Wrap -->

<!-- Region 2 Wrap -->

<div class="container region2wrap">

  <div class="row">

    <!-- Logo -->

    <div class="three columns">

      <?php if ($logo): ?>
        <h1><a class="logo" href="<?php print $front_page; ?>"><img src="<?php print $logo; ?>" alt="<?php print t('Home'); ?>" /></a></h1>
      <?php endif; ?>
      <?php if ($site_name || $site_slogan): ?>
        <a href="<?php print $front_page; ?>" id="logo"><h1><?php print $site_name; ?></h1></a>
      <?php endif; ?>

    </div>

    <!-- End Logo -->

    <!-- Main Navigation -->

    <div class="nine columns">

      <nav class="top-bar">

        <ul>
          <!-- Toggle Button Mobile -->
          <li class="name">
            <h1><a href="#"> <?php print t('Please select your page'); ?></a></h1>
          </li>
          <li class="toggle-topbar"><a href="#"></a></li>
          <!-- End Toggle Button Mobile -->
        </ul>


        <?php if (!empty($navigation)): ?>
          <section><!-- Nav Section -->

            <?php print $navigation; ?>
          </section><!-- End Nav Section -->

        </nav>            
      <?php endif; ?>
    </div>

    <!-- End Main Navigation -->         

  </div>

</div>

<!-- End Region 2 Wrap -->


<?php if ($page['highlighted']): ?>
  <!-- Region 3 Wrap -->

  <div class="container region3wrap">  

    <?php print render($page['highlighted']); ?>

    <?php if (drupal_is_front_page()): ?>
      <!-- Content Top -->  
      <div class="row content_top">
        <div class="twelve columns">
          <h2><?php print theme_get_setting('home_tagline', 'touchm'); ?></h2>
        </div>
      </div>
      <!-- End Content Top -->
    <?php endif; ?>

  </div>

  <!-- End Region 3 Wrap -->

<?php endif; ?>

<?php if (!drupal_is_front_page()): ?>
  <!-- Region 3 Wrap -->

  <div class="container region3wrap">  


    <!-- Content Top -->  
    <div class="row content_top">

      <div class="nine columns">
        <?php if ($breadcrumb): ?>
          <?php print $breadcrumb; ?>
        <?php endif; ?>

      </div>

      <?php if (!empty($seach_block_form)): ?>
        <div class="three columns">

          <div class="row">
            <div class="twelve columns">
              <div class="row collapse top_search">
                <?php print $seach_block_form; ?>
              </div>
            </div>
          </div>

        </div>
      <?php endif; ?>
    </div>
    <!-- End Content Top -->

  </div>

  <!-- End Region 3 Wrap -->
<?php endif; ?>


<!-- Region 4 Wrap -->

<div class="container region4wrap">

  <div class="row maincontent">
    <?php if ($title): ?>
      <div class="twelve columns">

        <div class="page_title">
          <div class="row">
            <div class="twelve columns">
              <h1><?php print $title; ?></h1>
            </div>
          </div>
        </div>
      </div>
    <?php endif; ?>

    <?php if ($page['contact_map']): ?>
      <div class="twelve columns">
        <div class="map_location">
          <div class="map_canvas">
            <?php print render($page['contact_map']); ?>
          </div>
        </div>
      </div>
    <?php endif; ?>


    <!-- Main Content ( Middle Content) -->
    <?php
    $content_class = 'main-content';
    if ($page['sidebar_second'] || $page['sidebar_first']) {
      if ($page['sidebar_first']) {
        $content_class = 'eight columns push-four';
      } else {
        $content_class = 'eight columns';
      }
    }
    ?>
    <div class="clearfix"></div>
    <div id="content" class="<?php print $content_class; ?>">
      <?php print $messages; ?>
      <?php if (!empty($tabs['#primary']) || !empty($tabs['#secondary'])): ?>
        <div class="tabs">
          <?php print render($tabs); ?>
        </div>
      <?php endif; ?>
      <?php print render($page['help']); ?>
      <?php if ($action_links): ?>
        <ul class="action-links">
          <?php print render($action_links); ?>
        </ul>
      <?php endif; ?>
      <?php print render($page['content']); ?>
      <?php print $feed_icons; ?>
    </div>

    <?php if ($page['sidebar_second']): ?>
      <div class="four columns sidebar-right">
        <?php print render($page['sidebar_second']); ?>
      </div>
    <?php endif; ?>

    <?php if ($page['sidebar_first']): ?>
      <div class="four columns pull-eight sidebar-left">
        <?php print render($page['sidebar_first']); ?>
      </div>
    <?php endif; ?>

    <!-- End Main Content ( Middle Content) -->

  </div>
</div>

<!-- End Region 4 Wrap -->

<!-- Region 9 Wrap -->

<div class="container region9wrap">

  <?php if ($page['newsletter']): ?>
    <!-- Bottom Content -->
    <div class="row content_bottom">

      <?php print render($page['newsletter']); ?> 

    </div>
    <!-- End Bottom Content -->
  <?php endif; ?>


  <?php if ($page['footer_firstcolumn'] || $page['footer_secondcolumn'] || $page['footer_thirdcolumn']): ?>
    <!-- Footer -->  
    <div class="row footer">

      <!-- // // // // // // // // // // -->

      <div class="four columns">
        <?php print render($page['footer_firstcolumn']); ?>
      </div>

      <!-- // // // // // // // // // // -->

      <div class="four columns">
        <?php print render($page['footer_secondcolumn']); ?>

      </div>

      <!-- // // // // // // // // // // -->

      <div class="four columns">

        <?php print render($page['footer_thirdcolumn']); ?>

      </div>

      <!-- // // // // // // // // // // -->

    </div>
    <!-- End Footer -->
  <?php endif; ?>

</div>

<!-- End Region 9 Wrap -->

<!-- Region 10 Wrap -->

<div class="container region10wrap">

  <div class="row footer_bottom">

    <!-- Bottom -->

    <!-- // // // // // // // // // // -->

    <div class="six columns">

      <p class="copyright"><?php print theme_get_setting('footer_copyright_message', 'touchm'); ?></p>

    </div>

    <!-- // // // // // // // // // // -->

    <div class="six columns">

      <?php print render($page['footer']); ?>

    </div>

    <!-- // // // // // // // // // // -->

    <!-- Bottom -->

  </div>
</div>

<!-- End Region 10 Wrap -->



<!-- Back To Top -->
<a href="#" class="scrollup"><?php print t('Scroll'); ?></a>
<!-- End Back To Top -->

<!-- Reveal Modal --> 
<div id="Login" class="reveal-modal login-modal">

  <h2>Login</h2>
  <p>Please login using your credentials recived by email when you register.</p>
  <form>
    <label>Username</label>  
    <input type="text" placeholder="UserName@TouchM.com" />

    <label>Password</label>
    <input type="text" placeholder="insert password" />

    <p class="right"><a href="http://forums.orpalis.com/ucp.php?mode=sendpassword">I forgot my password</a> | <a href="http://forums.orpalis.com/ucp.php?mode=resend_act">Resend activation e-mail</a></p>

    <input type="submit" class="medium button" value="Login!">

  </form>

  <a class="close-reveal-modal">&#215;</a>

</div>   
<!-- End Reveal Modal -->
