<div class="twelve columns">

  <div class="row">

    <div class="four columns">

      <h4><?php print t('Overview'); ?></h4>

      <?php
      print render($content['body']);
      ?>

      <?php if (!empty($node->field_portfolio_link[LANGUAGE_NONE][0]['value'])): ?>
        <a href="<?php print url($node->field_portfolio_link[LANGUAGE_NONE][0]['value']); ?>" class="button"><?php print t('Launch Project'); ?></a>
      <?php endif; ?>

      <?php
      // We hide the comments and links now so that we can render them later.
      hide($content['comments']);
      hide($content['links']);
      hide($content['field_tags']);
      hide($content['field_portfolio_image']);
      hide($content['field_portfolio_category']);
      hide($content['field_portfolio_link']);


      print render($content);
      ?>

      <div class="clearfix"></div>
      <div class="project-pagination">
        <?php
        $prev_project_id = db_query("SELECT n.nid FROM {node} n WHERE n.type = :type AND n.status = 1 AND n.nid < :nid ORDER BY n.nid DESC", array(":type" => "portfolio", ":nid" => $node->nid))->fetchField();
        $next_project_id = db_query("SELECT n.nid FROM {node} n WHERE n.type = :type AND n.status = 1 AND n.nid > :nid ORDER BY n.nid ASC", array(":type" => "portfolio", ":nid" => $node->nid))->fetchField();
        ?>
        <?php if (!empty($prev_project_id)): ?>
          <a title="<?php print t('Previous Project'); ?>" href="<?php print url('node/' . $prev_project_id); ?>" class="has-tipsy left_pagination"></a>
        <?php endif; ?>
        <a title="<?php print t('View All Projects'); ?>" href="<?php print url('portfolio'); ?>" class="has-tipsy all_pagination"></a>
        <?php if (!empty($next_project_id)): ?>
          <a title="<?php print t('Next Project'); ?>" href="<?php print url('node/' . $next_project_id); ?>" class="has-tipsy right_pagination"></a>
        <?php endif; ?>

      </div>

    </div>

    <div class="eight columns">
      <?php print render($content['field_portfolio_image']); ?>

    </div>

  </div>

</div>