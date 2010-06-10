<?php
// $Id$

/**
 * Condition form: User has code.
 */
function w_condition_user_has_code_form($settings, &$form, &$form_state) {
  $form['settings']['user_code'] = array(
    '#type' => 'textfield',
    '#title' => t('User code'),
    '#default_value' => isset($settings['user_code']) ? $settings['user_code'] : '',
    '#description' => t('Leave blank if you would not like check exactly which code user has.'),
  );
}

/**
 * Condition form: User has profile field value.
 */
function w_condition_user_profile_field_form($settings, &$form, $form_state) {
  $form['settings']['field'] = array(
    '#type' => 'radios',
    '#title' => t('Field'),
    '#requried' => TRUE,
    '#options' => array(),
  );

  $q = db_query(
    'SELECT title, name, type, category, fid, weight'
      . ' FROM {profile_fields}'
      . ' WHERE 1'
      . ' ORDER BY category, weight'
  );

  while ($field = db_fetch_object($q)) {
    $form['settings']['field']['#options'][$field->name] = "{$field->name} ({$field->category})";
  }
}
