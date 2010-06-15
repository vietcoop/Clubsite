<?php
// $Id$

/**
 * @file
 * Helper functions for user functionalities.
 */

/**
 * @see w_init().
 * @see w_user().
 * @see w_user_first_time_login_settings_form().
 */
function w_user_first_time_login() {
  // Get message for user's first time login 
  $first_time_login_message = variable_get(
    'wUserFirstTimeLoginMessage',
    W_FIRST_TIME_LOGIN_MESSAGE
  );

  // Display message to user.
  drupal_set_message($first_time_login_message);

  // unset($_SESSION['w_first_time_login']);
}

/**
 * Users who have been inactive for 45 days or more are downgraded to Inactive User 
 * and sent an e-mail (built on a administrative-side template) letting them
 * know that they are welcome back on the forum.
 * 
 * @see w_user_inactivation_find().
 */
function w_user_inactivation_cron() {
  $rids = variable_get('wUserInactivationRolesToRemove', array());
  if (!empty($roles_removed)) {
    foreach (w_user_inactivation_find() as $uid) {
      $account = user_load($uid);

      if (!empty($account)) {
        w_user_inactivation_action($account, $rids);
      }
    }
  }
}

/**
 * Remove inactive user role.
 */
function w_user_inactivation_action($account, $rids) {
  $save = FALSE;
  foreach ($rids as $rid) {
    if (isset($account->roles[$rid])) {
      $save = TRUE;
      unset($account->roles[$rid]);
    }
  }

  if ($save) {
    user_save($account);
  }
}

/**
 * Find inactive users.
 * 
 * @return array $uids ID of users.
 */
function w_user_inactivation_find() {
  $sql =  "SELECT u.uid";
  $sql .= " FROM {users} u";
  $sql .= " WHERE uid <> 0 AND access <= %s";

  $user_inactive_time = variable_get('wUserInactivation', W_USER_INACTIVATION_DAYS);
  $user_inactive_time *= 24 * 60 * 60;
  $q = db_query($sql, time() - $user_inactive_time);

  while ($v = db_fetch_object($q)) {
    $uids[] = $v->uid;
  }

  return $uids;
}
