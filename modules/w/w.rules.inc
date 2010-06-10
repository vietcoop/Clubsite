<?php
// $Id$

/**
 * Implementation of hook_rules_condition_info().
 */
function w_rules_condition_info() {
  return array(
    'w_condition_user_has_code' => array(
      'label' => t('User has code'),
      'arguments' => array(
        'user' => array('type' => 'user', 'label' => t('User')),
      ),
      'help' => t('Whether the user has code'),
      'module' => 'User',
    ),
    'w_condition_user_profile_field' => array(
      'label' => t('User has field value'),
      'arguments' => array(
        'user' => array('type' => 'user', 'label' => t('User')),
      ),
      'help' => t('Whether the user has a field value'),
      'module' => 'User',
    ),
  );
}

/**
 * Implements of hook_rules_action_info().
 */
function w_rules_action_info() {
  return array(
    'w_action_remove_user_code' => array(
      'label' => t('Remove user code'),
      'arguments' => array(
        'user' => array('type' => 'user', 'label' => t('User whos code should be removed.')),
      ),
      'module' => 'User',
    ),
  );
}

/** 
 * Condition: User has code.
 */
function w_condition_user_has_code() {
}

/** 
 * Condition: User has profile field.
 */
function w_condition_user_profile_field($account, $settings) {
  dsm(serialize(!empty($account->{$settings['field']})));
  return !empty($account->{$settings['field']});
}

/**
 * Action: Remove user code.
 */
function w_action_remove_user_code($user, $settings) {
  module_load_include('inc', 'w', 'includes/w.code');
  return w_user_code_remove($user->uid);
}
