<?php
// $Id$

module_load_include('inc', 'w', 'includes/w.code');

/**
 * Helper function to alter to user-register form.
 * + Add UserCode field to form.
 * + Remove some profileFields form form.
 */
function _w_form_user_register_alter(&$form, &$form_state) {
  $use_code = $form['#parameters'][2];

  if ($use_code) {
    $field = array(
      '#type'   => 'textfield',
      '#weight' => W_USER_CODE_FIELD_WEIGHT,
      '#title'  => t('User code'),
      '#disabled' => TRUE,
      '#default_value' => 
        $use_code && sess_read(W_COOKIE_USER_CODE)
          ? sess_read(W_COOKIE_USER_CODE)
          : '',
    );

    $fieldset = W_USER_CODE_FIELDSET;
    if (!empty($fieldset)) {
      $form[W_USER_CODE_FIELDSET]['w_user_code'] = $field;
    }
    else {
      $form['w_user_code'] = $field;
    }
  }

  $settings = variable_get('wUserCodeFieldsVisibility', array());
  foreach (profile_categories() as $category) {
    foreach (element_children($form[$category['name']]) as $k) {
      if (in_array($k, array_keys($settings))) {
        switch ($settings[$k]) {
          case 'both':
            break;
          case 'none':
            unset($form[$category['name']][$k]);
            break;
          case $settings[$k] === 'use-code' && !$use_code:
          case $settings[$k] === 'not-use' && $use_code:
            unset($form[$category['name']][$k]);
            break;
        }
      }
    }
  }
}

function w_user_signup_callback($use_code = '') {
  if ($use_code === 'use-code' || $use_code === 'no-code') {
    if ($use_code === 'use-code') {
      $code = sess_read(W_COOKIE_USER_CODE);

      if (TRUE !== w_user_code_validate($code)) {
        return w_user_signup_callback();
      }
    }

    return drupal_get_form('user_register', $use_code === 'use-code');
  }

  return drupal_get_form('w_user_signup_user_code_form');
}

/**
 *
 */
function w_user_signup_user_code_form(&$form_state) {
  $form = array();

  $form['w_user_code'] = array(
    '#title' => t('User code'),
    '#type'  => 'textfield',
  );
  
  $form['continue'] = array(
    '#type'  => 'submit',
    '#value' => t('Use code'),
  );

  $form['skip'] = array(
    '#type' => 'submit',
    '#value' => t('Skip using code'),
  );
  
  return $form;
}

function w_user_signup_user_code_form_validate(&$form, &$form_state) {
  if ($form_state['clicked_button']['#value'] === t('Use code')) {
    $user_code_validation = w_user_code_validate($form_state['values']['w_user_code']);
    if (TRUE !== $user_code_validation) {
      form_set_error('', $user_code_validation);
    }
  }
}

function w_user_signup_user_code_form_submit(&$form, &$form_state) {
  if ($form_state['clicked_button']['#value'] === t('Use code')) {
    sess_write(W_COOKIE_USER_CODE, $form_state['values']['w_user_code']);
    $form_state['redirect'] = arg(0) === 'admin' ? 'admin/user/user/create/use-code' : 'user/register/use-code';
  }
  else {
    $form_state['redirect'] = arg(0) === 'admin' ? 'admin/user/user/create/no-code' : 'user/register/no-code';
  }
}
