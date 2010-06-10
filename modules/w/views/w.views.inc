<?php
// $Id$

/**
 * Implements hook_views_data().
 */
function w_views_data() {
  $data['w_user_code']['table']['group']  = t('User');
  $data['w_user_code']['table']['join'] = array(
    'users' => array(
      'handler' => 'views_join',
      'left_table' => 'users',
      'left_field' => 'uid',
      'field' => 'uid',
     ),
  );

  $data['w_user_code']['code'] = array(
    'title' => t('User code'),
    'help' => t('User code.'),
    'field' => array(
      'field' => 'code',
      'group' => t('User'),
      'handler' => 'views_handler_field_xss',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    // Information for accepting a title as a filter
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  return $data;
}
