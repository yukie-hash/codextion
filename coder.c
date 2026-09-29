/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: a81909 <a81909@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 18:55:07 by yhamada           #+#    #+#             */
/*   Updated: 2026/09/30 03:52:50 by a81909           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include<pthread.h>
#include<stdio.h>

typedef struct	s_coder
{
    int	id;
	int	compile_count;
}	t_coder;

void	*coder_routine(void	*arg){
	t_coder	*coder = (t_coder *)arg;

	while(1){
		// ドングル取得
		printf("%d is compiling\n", coder->id);
		usleep(200 * 1000); //コンパイル中
		// ドングル返却
		printf("%d is debugging\n",coder ->id);
		usleep(100 * 1000); //デバッグ中
		printf("%d is refactoring\n", coder->id);
		usleep(100 * 1000);
		coder->compile_count++;
	}
	return (NULL);
}

int	main(void){
	pthread_t	threads[3];
	t_coder		coders[3];
	int			i;

	// コーダーを３人作る
	i = 0;
	while(i < 3){
		coders[i].id = i + 1;  //なぜ.?
		coders[i].compile_count = 0;
		pthread_create(&threads[i], NULL, coder_routine, &coders[i]); //構造体を指すポインタを渡す
		i++;
	}

	i = 0;
	while(i < 3){
		pthread_join(threads[i], NULL); //スレッドの終了を待って回収
		i++;
	}
	return (0);
}
