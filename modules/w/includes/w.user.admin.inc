<?php
// $Id$

/**
 * @file
 * User management functions.
 */

/**
 * Settings form for user's first time login.
 */
function w_user_first_time_login_settings_form(&$form_state) {
  $form['wUserFirstTimeLoginMessage'] = array(
    '#type'  => 'textarea',
    '#title' => 'Messasge to display user on their first time login',
    '#required' => TRUE,
    '#default_value' => variable_get(
      'wUserFirstTimeLoginMessage',
      W_FIRST_TIME_LOGIN_MESSAGE
    ),
  );

  return system_settings_form($form);
}

/**
 * User inactivation settings form.
 */
function w_user_inactivation_settings_form(&$form_state) {
  $form['wUserInactivation'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => 'User inactivation time',
    '#size' => 2,
    '#field_suffix' => t('days'),
    '#default_value' => variable_get('wUserInactivation', W_USER_INACTIVATION_DAYS),
  );

  // Role to remove
  $roles = user_roles(TRUE);
  unset($roles[2]);
  $form['wUserInactivationRolesToRemove'] = array(
    '#type'     => 'checkboxes',
    '#title'    => 'Role(s) to be removed',
    '#required' => TRUE,
    '#options'  => $roles,
    '#default_value' => variable_get('wUserInactivationRolesToRemove', array()),
    '#description' => 'Users will be granted these roles immediately after logging in.',
  );

  return system_settings_form($form);
}

/**
 * Validation for user-inactivation settings form.
 */
function w_user_inactivation_settings_form_validate(&$form, &$form_state) {
  $v = &$form_state['values'];

  // Only accept number
  if (
    is_numeric($v['wUserInactivation']) &&
    (is_numeric($v['wUserInactivation']) && $v['wUserInactivation'] < 0)
  ) {
    form_set_error('wUserInactivation', t('Invalid value.'));
  }
}
