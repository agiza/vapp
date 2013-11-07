<?php

/**
 * @file
 * Amazon EC2 Library Common file
 *
 * Copyright (c) 2010-2012 DOCOMO Innovations, Inc.
 *
 */

/**
 * Retrieve instances information using REST call.
 *
 * @param $cloud_context
 *              The sub-cloud for which the information is to be retrieved
 * @return
 */
function aws_ec2_lib_fetch_instances_data($cloud_context) {
  if (_cloud_is_update_allowed()) {
    $result = aws_ec2_lib_fetch_data($cloud_context);
  }
  $dest = drupal_get_destination();
  drupal_goto( $dest );
}


/**
 * Fetch the volumes related data
 * @param $cloud_context
 *          The sub-cloud for which the information is to retrieved
 * @return
 */
function aws_ec2_lib_fetch_volumes_data($cloud_context) {
  if (_cloud_is_update_allowed()) {
    aws_ec2_lib_fetch_data($cloud_context);
  }
  drupal_goto( cloud_get_menu_path($cloud_context) . '/ebs_volumes' );
}


/**
 *
 * @param $cloud_context
 *              The sub-cloud for which the information is to be retrieved
 * @return
 */
function aws_ec2_lib_fetch_elastic_ips_data($cloud_context) {
  if (_cloud_is_update_allowed()) {
    aws_ec2_lib_fetch_data($cloud_context);
  }
  drupal_goto( cloud_get_menu_path($cloud_context) . '/elastic_ips' );
}


/**
 *
 * @param $cloud_context
 *              The sub-cloud for which the information is to be retrieved
 * @return
 */
function aws_ec2_lib_fetch_security_group_data($cloud_context) {
  if (_cloud_is_update_allowed()) {
    aws_ec2_lib_fetch_data($cloud_context);
  }
  drupal_goto( cloud_get_menu_path($cloud_context) . '/security_groups' );
}


/**
 *
 * @param $cloud_context
 *              The sub-cloud for which the information is to be retrieved
 * @return
 */
function aws_ec2_lib_fetch_snapshots_data($cloud_context) {
  if (_cloud_is_update_allowed()) {
    aws_ec2_lib_fetch_data($cloud_context);
  }
  drupal_goto( cloud_get_menu_path($cloud_context) . '/ebs_snapshots' );
}


/**
 *
 * @param $cloud_context
 *              The sub-cloud for which the information is to be retrieved
 * @return
 */
function aws_ec2_lib_fetch_ssh_keys_data($cloud_context) {
  if (_cloud_is_update_allowed()) {
    aws_ec2_lib_fetch_data($cloud_context);
  }
  drupal_goto( cloud_get_menu_path($cloud_context) . '/ssh_keys' );

}


/**
 *
 * Send request and fetch all the information
 * The purpose of this function is to download all the available information from the sub-cloud
 * HTTP Request is to the sub-cloud server and the information returned is stored in the database
 * It is necessary to download the latest information to keep clanavi updated about the latest information.
 * Information requested is mainly of the following entities:
 *
 *    viz.  Elastic IP Info,
 *          Instances Info,
 *          SSH Keys Info,
 *          Security Group Info,
 *          Zone Info,
 *          Snapshot Info,
 *          Volume Info
 *
 *
 * @param $cloud_context
 *              The sub-cloud for which the information is to be retrieved
 * Baldwin - Cron based download
 */
function aws_ec2_lib_fetch_data($cloud_context) {

  drupal_set_time_limit(AWS_EC2_LIB_TIME_LIMIT);
  return _aws_ec2_lib_download_all($cloud_context);
}


/**
 * Retrieve information related to images
 *
 * @param $cloud_context
 *                  The sub-cloud for which the information is to be retrieved
 */
function aws_ec2_lib_fetch_image_data($cloud_context) {

  drupal_set_time_limit(AWS_EC2_LIB_TIME_LIMIT);
  _aws_ec2_lib_download_all_images($cloud_context);
  $destination = drupal_get_destination();

  drupal_goto( $destination );
}


/**
 *
 * @param $cloud_context
 *              The sub-cloud for which the information is to be retrieved
 * @return
 */
function aws_ec2_lib_get_zones($cloud_context) {

  $zone_options = array();
  $zone_options['any'] = 'any';

  $query = _aws_ec2_lib_get_describe_zones_query($cloud_context);


  $zones_result = db_query( $query, array(':cloud_type' => $cloud_context) );


  foreach ($zones_result as $type) {

    $zone_options[$type->zone_name] = $type->zone_name;
  }
  asort( $zone_options);

  return $zone_options;
}


/**
 * This function defines the schema for storing the Cloud related information
 *
 * @param $cloud_context
 *              The sub-cloud for which the schema is to be created
 * @return
 *      schema for the sub-cloud
 */
function aws_ec2_lib_get_schema($cloud_context) {

  $ec2_lib_instances_table                  = aws_ec2_lib_get_table_info(AWS_EC2_LIB_INSTANCES_TABLE             );
  $ec2_lib_instances_lock_table             = aws_ec2_lib_get_table_info(AWS_EC2_LIB_INSTANCES_LOCK_TABLE        );
  $ec2_lib_images_table                     = aws_ec2_lib_get_table_info(AWS_EC2_LIB_IMAGES_TABLE                );
  $ec2_lib_ssh_keys_table                   = aws_ec2_lib_get_table_info(AWS_EC2_LIB_SSH_KEYS_TABLE              );
  $ec2_lib_security_groups_table            = aws_ec2_lib_get_table_info(AWS_EC2_LIB_SECURITY_GROUPS_TABLE       );
  $ec2_lib_security_groups_details_table    = aws_ec2_lib_get_table_info(AWS_EC2_LIB_SECURITY_GROUPS_DETAILS_TABLE );
  $ec2_lib_zone_table                       = aws_ec2_lib_get_table_info(AWS_EC2_LIB_ZONE_TABLE                  );
  $ec2_lib_elastic_ip_table                 = aws_ec2_lib_get_table_info(AWS_EC2_LIB_ELASTIC_IP_TABLE            );
  $ec2_lib_snapshot_table                   = aws_ec2_lib_get_table_info(AWS_EC2_LIB_SNAPSHOTS_TABLE             );
  $ec2_lib_volumes_table                    = aws_ec2_lib_get_table_info(AWS_EC2_LIB_EBS_VOLUMES_TABLE           );
  $ec2_lib_volumes_device_opt_table         = aws_ec2_lib_get_table_info(AWS_EC2_LIB_VOLUMES_DEVICE_OPTIONS_TABLE);
  $ec2_lib_user_keys_table                  = aws_ec2_lib_get_table_info(AWS_EC2_LIB_USER_KEYS_TABLE             );
  $ec2_lib_register_image_table             = aws_ec2_lib_get_table_info(AWS_EC2_LIB_REGISTER_IMAGE_STATUS_TABLE );

  $schema = array();

  $schema[$ec2_lib_instances_table] = array(
    'description' => t('EC2 Instances'),
    'fields' => array(
      'instance_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'cloud_type' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'instance_nickname' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'image_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'kernel_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'ramdisk_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'owner_id' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'group_id' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'reservation_id' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'platform' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'instance_type' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'placement' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'instance_state_code' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'private_dns_name' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'dns_name' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'key_name' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'instance_state_name' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'reason' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'ami_launch_index' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'product_codes' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'monitoring' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'launch_time' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
    ),
    'primary key' => array('instance_id'),
  );

  $schema[$ec2_lib_images_table] = array(
    'description' => t('EC2 Images'),
    'fields' => array(
      'image_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'cloud_type' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'image_nickname' => array(
        'type' => 'varchar',
        'length' => 256,
      ),
      'architecture' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'kernel_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'ramdisk_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'imageowner_id' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'image_type' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'region' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'image_location' => array(
        'type' => 'varchar',
        'length' => 256,
      ),
      'image_state' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'is_public' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
    ),
    'primary key' => array('image_id'),
  );

  $schema[$ec2_lib_volumes_table] = array(
    'description' => t('Volume'),
    'fields' => array(
      'volume_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'cloud_type' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'nickname' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'snapshot_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'instance_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'vol_size' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'vol_zone' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'device' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'status' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'created' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'attached' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
    ),
    'primary key' => array('volume_id'),
  );

  $schema[$ec2_lib_elastic_ip_table] = array(
    'description' => t('Elastic IP'),
    'fields' => array(
      'cloud_type' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'public_ip' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'instance_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'public_ip_name' => array(
        'type' => 'varchar',
        'length' => 128,
      ),
    ),
    'primary key' => array('public_ip'),
  );

  $schema[$ec2_lib_instances_lock_table] = array(
    'description' => t('Instance Information'),
    'fields' => array(
      'instance_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'cloud_type' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'monitor' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'script_status' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'is_locked' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'ssh_user_name' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
    ),
    'primary key' => array('instance_id'),
  );

  $schema[$ec2_lib_register_image_table] = array(
    'description' => t('EC2 Instance Types'),
    'fields' => array(
      'id' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'cloud_type' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'image_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'owner' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'status' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
    ),
    'primary key' => array('id'),
  );

  $schema[$ec2_lib_security_groups_table] = array(
    'description' => t('EC2 Security Groups'),
    'fields' => array(
      'owner_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'cloud_type' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'group_name' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'group_id' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'group_description' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'permissions' => array(
        'type' => 'varchar',
        'length' => 1280,
      ),
    ),
    'primary key' => array('owner_id', 'cloud_type', 'group_name'),
  );

  $schema[$ec2_lib_security_groups_details_table] = array(
    'description' => t('EC2 Security Groups Details Table'),
    'fields' => array(
      'group_name' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'cloud_type' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'ip_protocol' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'from_port' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'to_port' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'user_id' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'tgt_group_name' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'cidr_ip' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
    ),
  );

  $schema[$ec2_lib_snapshot_table] = array(
    'description' => t('EC2 Snapshot'),
    'fields' => array(
      'snapshot_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'cloud_type' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'nickname' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'volume_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'owner_id' => array(
        'type' => 'varchar',
        'length' => 16,
      ),
      'volume_size' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'status' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'progress' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'description' => array(
        'type' => 'varchar',
        'length' => 256,
      ),
      'created' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
    ),
    'primary key' => array('snapshot_id'),
  );

  $schema[$ec2_lib_ssh_keys_table] = array(
    'description' => t('EC2 SSH Keys'),
    'fields' => array(
      'key_name' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'cloud_type' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'fingerprint' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'key_material' => array(
        'type' => 'varchar',
        'length' => 4096,
      ),
    ),
    'primary key' => array('key_name', 'cloud_type'),
  );


  $schema[$ec2_lib_user_keys_table] = array(
    'description' => t('User Keys'),
    'fields' => array(
      'key_name' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'cloud_type' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'owner' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
    ),
    'primary key' => array('key_name', 'cloud_type'),
  );


  $schema[$ec2_lib_zone_table] = array(
    'description' => t('Availability Zone'),
    'fields' => array(
      'region' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'zone_name' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'cloud_type' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
      'zone_state' => array(
        'type' => 'varchar',
        'length' => 64,
      ),
    ),
    'primary key' => array('zone_name', 'cloud_type'),
  );

  return $schema;
}


function aws_ec2_lib_has_instances_data($cloud_context) {

  if ( _aws_ec2_lib_get_security_groups_list_count($cloud_context) == 0
    ||     _aws_ec2_lib_get_ssh_keys_list_count(       $cloud_context) == 0 ) {

    return FALSE;
  }

  return TRUE;
}


function aws_ec2_lib_has_ssh_key_data($cloud_context) {

  if ( _aws_ec2_lib_get_ssh_keys_list_count( $cloud_context) == 0 ) {

    return FALSE;
  }

  return TRUE;
}

function aws_ec2_lib_has_security_group_data($cloud_context) {

  if ( _aws_ec2_lib_get_security_groups_list_count($cloud_context) == 0 ) {

    return FALSE;
  }

  return TRUE;
}

function aws_ec2_lib_cluster_form($cloud_context, $form, $params) {

  $cluster_id         = $params['cluster_id'      ]  ;
  $cluster_nickname   = $params['cluster_nickname'];

  $form                = array(); // New Form

  //pass in cloud_context
  $form['cloud_context'] = array(
    '#type' => 'value',
    '#value' => $cloud_context,
  );

  $form['fieldset_template_info'] = array('#type' => 'fieldset' /* '#title' => t('Template Info') */);

  $form['cluster_id']  = array(
    '#type' => 'hidden',
    '#value' => $cluster_id,
  );

  $form['attach_header'] = array(
    '#prefix' => '<h2>',
    '#value' => t('Cluster: @cluster_nickname', array('@cluster_nickname' => $cluster_nickname) ),
    '#suffix' => '</h2>',
  );

  $template_options = cloud_get_server_templates($cloud_context);
  $template_options['-1'] = ' - Select Template - '  ;

  $form['template_details'] = array(
    'template_label' => array(
      '#type' => 'item'    ,
      '#title' => t('Template'),
    ),
    'template_select' => array(
      '#type' => 'select'  ,
      '#options' => $template_options,
      '#default_value' => -1,
    ),
  );

  $form['nickname_details'] = array(
    'nickname_label' => array(
      '#type' => 'item'     ,
      '#title' => t('Nickname'),
    ),
    'nickname_text' => array(
      '#type' => 'textfield',
      '#default_value' => '',
    ),
  );

  $ssh_key_options = _aws_ec2_lib_get_ssh_keys($cloud_context);
  $form['ssh_keys_details'] = array(
    'ssh_keys_label' => array(
      '#type' => 'item'      ,
      '#title' => t('SSH Key'),
    ),
    'ssh_keys_select' => array(
      '#type' => 'select'    ,
      '#options' => $ssh_key_options,
    ),
  );

  $security_group_options = _aws_ec2_lib_get_security_groups($cloud_context);
  $form['SG_details'      ] = array(
    'SG_label' => array(
      '#type' => 'item'      ,
      '#title' => t('Security Group(s)'),
    ),
    'SG_select' => array(
      '#type' => 'select'    ,
      '#options' => $security_group_options,
      '#multiple' => TRUE,
    ),
  );

  $zone_options = aws_ec2_lib_get_zones($cloud_context);
  $form['zone_details'    ] = array(
    'zone_label' => array(
      '#type' => 'item'      ,
      '#title' => t('Availability Zone'),
    ),
    'zone_select' => array(
      '#type' => 'select'    ,
      '#options' => $zone_options,
    ),
  );

  $form['submitted'] = FALSE;

  $form['submit_buttons'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<span class="clear"></span><div class="container-inline"><div class="buttons">',
    '#suffix' => '</div></div>',
    'Add_more' => array(
      '#type' => 'submit',
      '#value' => t('Add More'     ),
    ),
    'Save' => array(
      '#type' => 'submit',
      '#value' => t('Save'         ),
    ),
    'Cancel' => array(
      '#type' => 'submit',
      '#value' => t('List Servers' ),
    ),
  );

  return $form;
}


/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_aws_ec2_lib_cluster_form($form) {


  if (isset($form['form'])) {
    $form = $form['form'];
  }

  $rows = array(
    array(
      drupal_render($form['template_details']['template_label']),
      drupal_render($form['template_details']['template_select']),
    ),
    array(
      drupal_render($form['nickname_details']['nickname_label']),
      drupal_render($form['nickname_details']['nickname_text']),
    ),
    array(
      drupal_render($form['ssh_keys_details']['ssh_keys_label']),
      drupal_render($form['ssh_keys_details']['ssh_keys_select']),
    ),
    array(
      drupal_render($form['SG_details']['SG_label']),
      drupal_render($form['SG_details']['SG_select']),
    ),
    array(
      drupal_render($form['zone_details']['zone_label']),
      drupal_render($form['zone_details']['zone_select']),
    ),
  );

  $table = theme('table', array('header' => NULL, 'rows' => $rows));
  $form['fieldset_template_info']['#children'] = $table;

  //cf. Waning by Coder module: Potential problem: when FAPI element '#type' is set to 'markup' (default), '#value' only accepts filtered text, be sure to use check_plain(), filter_xss() or similar to ensure your $variable is fully sanitized.
  // form['fieldset_template_info']['list'] = array('#type' =>
  // 'markup', '#value' => $table);

  $output  = drupal_render($form['attach_header'         ]);
  $output .= drupal_render($form['fieldset_template_info']);
  $output .= drupal_render($form['submit_buttons'        ]);
  $output .= drupal_render($form['cluster_id'            ]);
  $output .= drupal_render_children($form);

  return $output;
}


function aws_ec2_lib_cluster_form_validate($cloud_context, $form_id, $form_values) {

  $form_values = $form_values['values'];

  if ( $form_values['op'] == t('Save') || $form_values['op'] == t('Add More')  ) {

    if ($form_values['template_select'] == -1 ) {

      form_set_error('', t('You must select Template.'));
    }

    if (empty($form_values['nickname_text']) ) {

      form_set_error('', t('You must enter valid Nickname.'));
    }

    if (empty($form_values['ssh_keys_select'])
      ||          $form_values['ssh_keys_select'] == -1 ) {

      form_set_error('', t('You must enter valid SSH Key.'));
    }

    if (count($form_values['SG_select']) == 0 ) {

      form_set_error('', t('You must select Security Group.'));
    }
  }
}


function aws_ec2_lib_cluster_form_submit($cloud_context, $form_id, $form_values) {

  $form_values = $form_values['values'];

  if ( $form_values['op'] == t('Save'        )
    ||     $form_values['op'] == t('List Servers')
    ||     $form_values['op'] == t('Add More')   ) {
    cloud_callback_cluster_operation( 'cluster_form_submit', $form_id, $form_values );
  }

  return;
}




function aws_ec2_lib_process_bundle_instance() {

  $script_filename = dirname(__FILE__) . CLOUD_PATH_SEPARATOR . 'scripts/aws_ec2_lib_bundle_image_ami_tools.sh';
  $script_handle   = fopen( $script_filename, "r" );
  $contents        = fread($script_handle, filesize($script_filename) );
  fclose( $script_handle );

  $output_dir = file_create_path('cloud');

  if ( file_exists($output_dir) === FALSE ) {

    mkdir($output_dir, 0777, TRUE );
  }

  $inst_to_bundle = aws_ec2_lib_db_bundle_instance_get('initiated');

  foreach ($inst_to_bundle as $inst) {

    $cloud_context      = $inst['cloud_context']  ;
    $certificate        = cloud_get_info( $cloud_context, 'certificate'        );
    $image_upload_url   = cloud_get_info( $cloud_context, 'image_upload_url'   );
    $user_id            = cloud_get_info( $cloud_context, 'user_id'            );
    $access_key         = cloud_get_info( $cloud_context, 'aws_access_key'     );
    $secret_key         = cloud_get_info( $cloud_context, 'aws_secret_key'     );
    $image_register_url = cloud_get_info( $cloud_context, 'image_register_url' );
    $instance_id        = $inst['instance_id'];
    $vol_size           = $inst['volume_size'];
    $nickname           = $inst['nickname'   ];
    $bucketname         = $inst['bucketname'];
    $foldername         = $inst['foldername'];
    $imagename          = $inst['nickname'   ];
    $instance           = cloud_get_instance($cloud_context, $instance_id);


    if ($instance) {

      $ssh_user_name  = aws_ec2_lib_get_ssh_user( $cloud_context, $instance_id );
      $key_name       = $instance['key_name'];
      $instance_ip    = $instance['dns_name'];
      $dir            = $output_dir . CLOUD_PATH_SEPARATOR . $instance_id . CLOUD_PATH_SEPARATOR . 'bundle_instance';
      $command        = 'mkdir -p ' . $dir;
      exec($command, $output, $retval); //command for creating new folder
      $script_file    = $dir . CLOUD_PATH_SEPARATOR . 'bundle_instance.sh'  ;
      $cert_file      = $dir . CLOUD_PATH_SEPARATOR . 'cert.pem'  ;
      $perm_file      = _cloud_scripting_create_temp_permission_file($cloud_context,    $instance_id, $key_name);
      $script_str     = str_replace("\r\n",    "\n", $contents );

      // Update bucketname
      $nickname       = str_replace( array('-', '_', ':', ' '),    ''                   , $nickname );

      // Replace necessary parameters
      $script_str         = str_replace("@CLANAVI_BUCKET_NAME"          ,    $bucketname          , $script_str );
      $script_str         = str_replace("@CLANAVI_FOLDER_NAME"          ,    $foldername          , $script_str );
      $script_str         = str_replace("@CLANAVI_IMAGE_NAME"           ,    $imagename           , $script_str );
      $script_str         = str_replace("@CLANAVI_SIZE"                 ,    $vol_size            , $script_str );
      $script_str         = str_replace("@CLANAVI_AWS_USER_ID"          ,    $user_id             , $script_str );
      $script_str         = str_replace("@CLANAVI_AWS_ACCESS_KEY_ID"    ,    $access_key          , $script_str );
      $script_str         = str_replace("@CLANAVI_AWS_SECRET_ACCESS_KEY",    $secret_key          , $script_str );
      $script_str         = str_replace("@CLANAVI_IMAGE_UPLOAD_URL"     ,    $image_upload_url    , $script_str );
      $script_str         = str_replace("@CLANAVI_IMAGE_REGISTER_URL"   ,    $image_register_url  , $script_str );



      $fh = fopen($script_file, 'w');
      if ( $fh == FALSE) {
        drupal_set_message( t('Unable to create the script file.'), 'error' );
        continue;
      }

      fwrite($fh, $script_str );
      fclose($fh);

      $fh = fopen($cert_file, 'w');
      if ( $fh == FALSE) {

        drupal_set_message( t('Unable to create the certificate file.'), 'error' );
        continue;
      }

      fwrite($fh, $certificate );
      fclose($fh);

      $script_dest_path = '/tmp/clanavi/' . $instance_id . CLOUD_PATH_SEPARATOR . 'bundle_instance.sh';
      if ( _cloud_scripting_is_ssh_enabled($cloud_context, $instance_ip, $instance_id, $perm_file) == '0' ) {

        // Instance is not ready for ssh
        // Do not execute script right now
        continue;
      }

      // Execute the bundle instance
      $log_file_name     = CLOUD_SCRIPTING_TMP_LOG_FILE_PATH . $instance_id . '/' . REQUEST_TIME . '.log'  ;
      // Update status
      aws_ec2_lib_db_bundle_instance_update_status($instance_id, 'processed');

      $script_id = 'bundle_instance';

      $tmp_args  = ' ' . $instance_ip . ' ' . $perm_file . ' ' . $log_file_name . ' ' . "\"" . $script_file . "\"" .
                  ' ' . $cert_file . ' ' . $perm_file .
                  ' ' . $instance_id . ' ' . $script_id . ' ' . CLOUD_SSH_PATH . ' ' . CLOUD_SCP_PATH .
                  ' ' . CLOUD_SCRIPTING_TEMP_DIRECTORY . ' ' . CLOUD_SCRIPTING_OUTPUT_LOG_FILE . ' ' . $ssh_user_name .
                  ' ' . "\"" . $script_dest_path . "\""  ;

      $command     = CLOUD_PHP_PATH . ' -f ' . dirname(__FILE__) . CLOUD_PATH_SEPARATOR . 'aws_ec2_lib_bundle_instance_exec.inc ' . $tmp_args . ' >> /tmp/cloud.log'  ;

      exec( $command, $output, $retval);

    }
  }

  // Check the in progress bundle_instances
  $inst_to_bundle = aws_ec2_lib_db_bundle_instance_get('processed');

  foreach ($inst_to_bundle as $inst) {

    $cloud_context      = $inst['cloud_context']  ;
    $instance_id        = $inst['instance_id'];
    $instance           = cloud_get_instance($cloud_context, $instance_id);

    if ($instance) {

      $key_name         = $instance['key_name'];
      $instance_ip      = $instance['dns_name'];
      $dir              = $output_dir . CLOUD_PATH_SEPARATOR . $instance_id . CLOUD_PATH_SEPARATOR . 'bundle_instance';
      $command          = 'mkdir -p ' . $dir;
      exec($command, $output, $retval); //command for creating new empty folder
      $perm_file        = _cloud_scripting_create_temp_permission_file($cloud_context,    $instance_id, $key_name);

      $status           = 'success';
      $completed = _aws_ec2_lib_bundle_instance_check_status($cloud_context, $instance_ip, $instance_id, $perm_file, $status  );

      if ($completed) {

        aws_ec2_lib_db_bundle_instance_update_status($instance_id, $status);
        continue;
      }

      $status = 'failed';
      $failed = _aws_ec2_lib_bundle_instance_check_status($cloud_context, $instance_ip, $instance_id, $perm_file,    $status );
      if ($failed) {

        aws_ec2_lib_db_bundle_instance_update_status($instance_id, $status);
        continue;
      }

    }
  }

  // Registration Process
  $inst_to_bundle = aws_ec2_lib_db_bundle_instance_get('success');
  foreach ($inst_to_bundle as $inst) {

    $cloud_context = $inst['cloud_context']  ;
    $bucket        = $inst['bucketname'] . '/' . $inst['foldername'] . '/' . $inst['nickname'] . '.manifest.xml'  ;
    $myclient      = aws_ec2_api_register_image($cloud_context, $bucket);
    $obj_response  = new SimpleXMLElement($myclient->data);
    $instance_id   = $inst['instance_id'];

    if ( isset($obj_response->imageId) && empty($obj_response->imageId) == FALSE ) {

      $img_id = $obj_response->imageId;
      aws_ec2_lib_db_bundle_instance_update_status($instance_id, 'complete:' . $img_id );
      continue;
    }
    else {

      aws_ec2_lib_db_bundle_instance_update_status($instance_id, 'regisHter-failed');
      continue;
    }
  }

}



/**
 *
 * @param <type> $instance_ip
 * @param <type> $instance_id
 * @param <type> $perm_file
 * @param <type> $script_id
 * @return <type>
 */
function _aws_ec2_lib_bundle_instance_check_status($cloud_context, $instance_ip, $instance_id, $perm_file, $status ) {

  $ssh_user_name  = aws_ec2_lib_get_ssh_user( $cloud_context, $instance_id );
  $status_file    = '/tmp/clanavi/bundle_instance.' . $status;
  $remote_command = "[ -f $status_file ] && echo '1' || echo '0'";
  $command        = CLOUD_SSH_PATH . ' ' . $ssh_user_name . '@' . $instance_ip . ' -i "' . $perm_file . '" -o \'StrictHostKeyChecking no\' "' . $remote_command . '"';
  exec($command, $output, $retval);

  $result = isset($output[0]) ? $output[0] : 0;

  return $result;
}

